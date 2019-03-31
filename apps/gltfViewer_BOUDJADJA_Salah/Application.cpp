#define TINYGLTF_IMPLEMENTATION

#include "Application.hpp"

#include <iostream>
#include <vector>
#include <math.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glmlv/simple_geometry.hpp>
#include <glmlv/GLProgram.hpp>
#include <glmlv/ViewController.hpp>

using namespace std;
using namespace glmlv;

int Application::run()
{

    std::cout << "OpenGL Version : " << glGetString(GL_VERSION) << std::endl;
    /*--------------------------------------------*/
    float clearColor[3] = { 0, 0, 0 };
    
    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
        const auto viewportSize = m_GLFWHandle.framebufferSize();
        const auto seconds = glfwGetTime();
 	const auto projMatrix = glm::perspective(glm::radians(70.f), float(viewportSize.x) / viewportSize.y, 0.01f, 100.f);
        const auto viewMatrix = m_viewController.getViewMatrix();


        // Geometry pass
        
            m_geometryPassProgram.use();
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_GBufferFBO);

            glViewport(0, 0, m_nWindowWidth, m_nWindowHeight);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

 	    const auto mvMatrix = glm::translate(viewMatrix, glm::vec3(0.0f, 0.0f, -5.0f));
            const auto normalMatrix = glm::transpose(glm::inverse(mvMatrix));

            glUniformMatrix4fv(m_uModelViewProjMatrixLocation, 1, GL_FALSE, glm::value_ptr(projMatrix * mvMatrix));
            glUniformMatrix4fv(m_uModelViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvMatrix));
            glUniformMatrix4fv(m_uNormalMatrixLocation, 1, GL_FALSE, glm::value_ptr(normalMatrix));


            // Same sampler for all texture units
            glBindSampler(0, m_textureSampler);
     

            // Set texture unit of each sampler
            glUniform1i(m_uKdSamplerLocation, 0);


            glUniform3fv(m_uKdLocation, 1, glm::value_ptr(glm::vec3(1)));


	//On va dessiner en utilisant les vao produits à chaque primitive
	for (int i = 0; i < vaos.size(); ++i) {

			glBindVertexArray(vaos[i]);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textures[i]);

			tinygltf::Accessor indexAccessor = model.accessors[primitives[i].indices];
			glDrawElements(primitives[i].mode, indexAccessor.count, indexAccessor.componentType, (const void*) indexAccessor.byteOffset);
			
			glBindTexture(GL_TEXTURE_2D, 0);

			glBindVertexArray(0);
	}

	
         glBindSampler(0, m_textureSampler);
	
         glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

         glViewport(0, 0, viewportSize.x, viewportSize.y);
         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      if (m_CurrentlyDisplayed == GBufferTextureCount) // Beauty
        {
		
            // Shading pass
            {
                m_shadingPassProgram.use();

                glUniform3fv(m_uDirectionalLightDirLocation, 1, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(glm::normalize(m_DirLightDirection), 0))));
                glUniform3fv(m_uDirectionalLightIntensityLocation, 1, glm::value_ptr(m_DirLightColor * m_DirLightIntensity));

                glUniform3fv(m_uPointLightPositionLocation, 1, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(m_PointLightPosition, 1))));
                glUniform3fv(m_uPointLightIntensityLocation, 1, glm::value_ptr(m_PointLightColor * m_PointLightIntensity));

                for (int32_t i = GPosition; i < GDepth; ++i)
                {
                    glActiveTexture(GL_TEXTURE0 + i);
                    glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[i]);

                    glUniform1i(m_uGBufferSamplerLocations[i], i);
                }

                glBindVertexArray(m_TriangleVAO);
                glDrawArrays(GL_TRIANGLES, 0, 3);
                glBindVertexArray(0);
            }
        }
        else if (m_CurrentlyDisplayed == GDepth)
        {
            m_displayDepthProgram.use();

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GDepth]);

            glUniform1i(m_uGDepthSamplerLocation, 0);

            glBindVertexArray(m_TriangleVAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindVertexArray(0);
        }
        else if (m_CurrentlyDisplayed == GPosition)
        {
            m_displayPositionProgram.use();

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GPosition]);

            glUniform1i(m_uGDepthSamplerLocation, 0);

            const auto rcpProjMat = glm::inverse(projMatrix);

            const glm::vec4 frustrumTopRight(1, 1, 1, 1);
            const auto frustrumTopRight_view = rcpProjMat * frustrumTopRight;

            glUniform3fv(m_uSceneSizeLocation, 1, glm::value_ptr(frustrumTopRight_view / frustrumTopRight_view.w));

            glBindVertexArray(m_TriangleVAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindVertexArray(0);
        }
        else
        {
            // GBuffer display
            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_GBufferFBO);
            glReadBuffer(GL_COLOR_ATTACHMENT0 + m_CurrentlyDisplayed);
            glBlitFramebuffer(0, 0, m_nWindowWidth, m_nWindowHeight,
                0, 0, m_nWindowWidth, m_nWindowHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);

            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        }

   	// GUI code:
	glmlv::imguiNewFrame();

        {
            ImGui::Begin("GUI");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            if (ImGui::ColorEdit3("clearColor", clearColor)) {
                glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.f);
            }
            if (ImGui::CollapsingHeader("Directional Light"))
            {
                ImGui::ColorEdit3("DirLightColor", glm::value_ptr(m_DirLightColor));
                ImGui::DragFloat("DirLightIntensity", &m_DirLightIntensity, 0.1f, 0.f, 100.f);
                if (ImGui::DragFloat("Phi Angle", &m_DirLightPhiAngleDegrees, 1.0f, 0.0f, 360.f) ||
                    ImGui::DragFloat("Theta Angle", &m_DirLightThetaAngleDegrees, 1.0f, 0.0f, 180.f)) {
                    m_DirLightDirection = computeDirectionVector(glm::radians(m_DirLightPhiAngleDegrees), glm::radians(m_DirLightThetaAngleDegrees));
                }
            }

            if (ImGui::CollapsingHeader("Point Light"))
            {
                ImGui::ColorEdit3("PointLightColor", glm::value_ptr(m_PointLightColor));
                ImGui::DragFloat("PointLightIntensity", &m_PointLightIntensity, 0.1f, 0.f, 16000.f);
                ImGui::InputFloat3("Position", glm::value_ptr(m_PointLightPosition));
            }

            if (ImGui::CollapsingHeader("GBuffer"))
            {
                for (int32_t i = GPosition; i <= GBufferTextureCount; ++i)
                {
                    if (ImGui::RadioButton(m_GBufferTexNames[i], m_CurrentlyDisplayed == i))
                        m_CurrentlyDisplayed = GBufferTextureType(i);
                }
            }

            ImGui::End();
        }
        
        glmlv::imguiRenderFrame();

        
        /* Poll for and process events */
        glfwPollEvents();
        
        // Update the display
	m_GLFWHandle.swapBuffers(); // Swap front and back buffers

        auto ellapsedTime = glfwGetTime() - seconds;
        auto guiHasFocus = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
        if (!guiHasFocus) {
            m_viewController.update(float(ellapsedTime));
        }

    }


    return 0;
}

Application::Application(int argc, char** argv):
    m_AppPath { glmlv::fs::path{ argv[0] } },
    m_AppName { m_AppPath.stem().string() },
    m_ImGuiIniFilename { m_AppName + ".imgui.ini" },
    m_ShadersRootPath { m_AppPath.parent_path() / "shaders" },
    m_AssetsRootPath { m_AppPath.parent_path() / "assets" }

{
    	ImGui::GetIO().IniFilename = m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file
    
        //PROGRAM
	m_geometryPassProgram = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "geometryPass.vs.glsl", m_ShadersRootPath / m_AppName / "geometryPass.fs.glsl" });

	m_shadingPassProgram = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "shadingPass.vs.glsl", m_ShadersRootPath / m_AppName / "shadingPass.fs.glsl" });

	m_displayDepthProgram = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "shadingPass.vs.glsl", m_ShadersRootPath / m_AppName / "displayDepth.fs.glsl" });

	m_displayPositionProgram = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "shadingPass.vs.glsl", m_ShadersRootPath / m_AppName / "displayPosition.fs.glsl" });


	//RECUPERATION VARIABLES UNIFORMES
    m_uModelViewProjMatrixLocation = glGetUniformLocation(m_geometryPassProgram.glId(), "uModelViewProjMatrix");
    m_uModelViewMatrixLocation = glGetUniformLocation(m_geometryPassProgram.glId(), "uModelViewMatrix");
    m_uNormalMatrixLocation = glGetUniformLocation(m_geometryPassProgram.glId(), "uNormalMatrix");

    m_uKdLocation = glGetUniformLocation(m_geometryPassProgram.glId(), "uKd");

    m_uKdSamplerLocation = glGetUniformLocation(m_geometryPassProgram.glId(), "uKdSampler");

    m_uGBufferSamplerLocations[GPosition] = glGetUniformLocation(m_shadingPassProgram.glId(), "uGPosition");
    m_uGBufferSamplerLocations[GNormal] = glGetUniformLocation(m_shadingPassProgram.glId(), "uGNormal");
    m_uGBufferSamplerLocations[GDiffuse] = glGetUniformLocation(m_shadingPassProgram.glId(), "uGDiffuse");
    
    m_uDirectionalLightDirLocation = glGetUniformLocation(m_shadingPassProgram.glId(), "uDirectionalLightDir");
    m_uDirectionalLightIntensityLocation = glGetUniformLocation(m_shadingPassProgram.glId(), "uDirectionalLightIntensity");

    m_uPointLightPositionLocation = glGetUniformLocation(m_shadingPassProgram.glId(), "uPointLightPosition");
    m_uPointLightIntensityLocation = glGetUniformLocation(m_shadingPassProgram.glId(), "uPointLightIntensity");

    m_uGDepthSamplerLocation = glGetUniformLocation(m_displayDepthProgram.glId(), "uGDepth");
    m_uGPositionSamplerLocation = glGetUniformLocation(m_displayPositionProgram.glId(), "uGPosition");
    m_uSceneSizeLocation = glGetUniformLocation(m_displayPositionProgram.glId(), "uSceneSize");

	//Pour le glEnableVertexAttribArray et le glVertexAttribPointer
	const GLuint VERTEX_ATTR_POSITION = 0;
	const GLuint VERTEX_ATTR_NORMAL = 1;
	const GLuint VERTEX_ATTR_TEXCOORDS = 2;
	const GLuint VERTEX_ATTR_TANGENT = 3;

	//map pour faire correspondre les champs du gltf avec nos VERTEX_ATTR_POSITION,NORMAL etc..
	attribNb.insert({"POSITION", VERTEX_ATTR_POSITION});
	attribNb.insert({"NORMAL", VERTEX_ATTR_NORMAL});
	attribNb.insert({ "TANGENT", VERTEX_ATTR_TANGENT });
	attribNb.insert({ "TEXCOORD_0", VERTEX_ATTR_TEXCOORDS });

	//map pour faire correspondre les champs TINYGLTF_TYPE_VEC2 et TINYGLTF_TYPE_VEC3 du gltf avec les nombres 2 et 3 pour les vecteurs
	nbVec.insert({TINYGLTF_TYPE_VEC2, 2});
	nbVec.insert({TINYGLTF_TYPE_VEC3, 3});
       
	//chargement du gltf
	bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, argv[1]);

	if (!warn.empty()) {
		printf("Warn: %s\n", warn.c_str());
	}

	if (!err.empty()) {
		printf("Err: %s\n", err.c_str());
	}

	if (!ret) {
		printf("Failed to parse glTF\n");
	}


	//On va parcourir chaque primitive de chaque mesh du gltf
	//On aura un vao et un ibo par primitive
	//On aura un vbo par attribut d'une primitive
	for (int i = 0; i < model.meshes.size(); ++i) {
		for (int j = 0; j < model.meshes[i].primitives.size(); ++j) {

			//On cherche l'accesseur lié à la primitive
			tinygltf::Accessor indexAccessor = model.accessors[model.meshes[i].primitives[j].indices];
			//On cherche le bufferView auquel notre accesseur est relié
			tinygltf::BufferView bufferView = model.bufferViews[indexAccessor.bufferView];
			//On cherche à quel buffer est lié le bufferView
			int bufferIndex = bufferView.buffer;

			GLuint iboId;
 			glGenBuffers(1, &iboId);

			//On sélection la partie du buffer qui nous intéresse pour notre tableau d'index
  			std::vector<unsigned char>::const_iterator first = model.buffers[bufferIndex].data.begin() + bufferView.byteOffset;
  			std::vector<unsigned char>::const_iterator last = model.buffers[bufferIndex].data.begin() + bufferView.byteOffset + bufferView.byteLength;
  			std::vector<unsigned char> dataIndex(first, last);

		       	// Fill IBO
        		glBindBuffer(GL_ARRAY_BUFFER, iboId);
        		glBufferStorage(GL_ARRAY_BUFFER, dataIndex.size()* sizeof(unsigned char), dataIndex.data(), 0);
        		glBindBuffer(GL_ARRAY_BUFFER, 0);

			// VAO
			GLuint vaoId;
			glGenVertexArrays(1, &vaoId);
			glBindVertexArray(vaoId);


			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);


			//On regarde chaque attribut de la primitive
			for (std::map<std::string, int>::iterator it = model.meshes[i].primitives[j].attributes.begin(); it != model.meshes[i].primitives[j].attributes.end(); ++it) {
				//on ignore l'attribut TANGENT
				if (it->first != "TANGENT") {

					tinygltf::Accessor accessor = model.accessors[model.meshes[i].primitives[j].attributes[it->first]];
					bufferView = model.bufferViews[accessor.bufferView];
					bufferIndex = bufferView.buffer;

					//On sélection la partie du buffer qui nous intéresse pour notre vbo
					std::vector<unsigned char>::const_iterator first1 = model.buffers[bufferIndex].data.begin() + bufferView.byteOffset;
  					std::vector<unsigned char>::const_iterator last1 = model.buffers[bufferIndex].data.begin() + bufferView.byteOffset + bufferView.byteLength;
  					std::vector<unsigned char> dataVbo(first1, last1);

					GLuint vbo;
    					glGenBuffers(1, &vbo);

					// Fill VBO
        				glBindBuffer(GL_ARRAY_BUFFER, vbo);
        				glBufferStorage(GL_ARRAY_BUFFER, dataVbo.size()* sizeof(unsigned char), dataVbo.data(), 0);
        				glBindBuffer(GL_ARRAY_BUFFER, 0);


					glBindBuffer(GL_ARRAY_BUFFER, vbo);
					glEnableVertexAttribArray(attribNb[it->first]); 
					glVertexAttribPointer(attribNb[it->first], nbVec[accessor.type], accessor.componentType, GL_FALSE, bufferView.byteStride, (const GLvoid*)(accessor.byteOffset)); 
					glBindBuffer(GL_ARRAY_BUFFER, 0);
				}


			}

			vaos.push_back(vaoId);
			vaos.size();
			primitives.push_back(model.meshes[i].primitives[j]);


			//TEXTURE
			tinygltf::Material material = model.materials[model.meshes[i].primitives[j].material];

			if(material.values.find("baseColorTexture") != material.values.end() ){


				const auto& param = material.values["baseColorTexture"];
				tinygltf::Texture tex = model.textures[param.TextureIndex()];
				tinygltf::Image &image = model.images[tex.source];

  				glActiveTexture(GL_TEXTURE0);
       				// Upload all textures to the GPU

       				glmlv::Image2DRGBA image2 = glmlv::readImage(m_AssetsRootPath / m_AppName / "models" / image.uri);


            			GLuint texId = 0;
            			glGenTextures(1, &texId);
           			glBindTexture(GL_TEXTURE_2D, texId);

            			glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, image2.width(), image2.height());
            			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image2.width(), image2.height(), GL_RGBA, GL_UNSIGNED_BYTE,  image2.data());

            			glBindTexture(GL_TEXTURE_2D, 0);

				textures.push_back(texId);

			}
			else{

				//texture par défaut blanche il n'y a pas de champs baseColorTexture ds le gltf

				glActiveTexture(GL_TEXTURE0);
       				// Upload all textures to the GPU

            			GLuint texId = 0;
            			glGenTextures(1, &texId);
           			glBindTexture(GL_TEXTURE_2D, texId);

            			glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, 1, 1);
        			glm::vec4 white(1.f, 1.f, 1.f, 1.f);
        			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_FLOAT, &white);

				glBindTexture(GL_TEXTURE_2D, 0);

				textures.push_back(texId);

			}
			

		


		}
	}


       glGenSamplers(1, &m_textureSampler);
       glSamplerParameteri(m_textureSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
       glSamplerParameteri(m_textureSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

 	// Init GBuffer
    glGenTextures(GBufferTextureCount, m_GBufferTextures);

    for (int32_t i = GPosition; i < GBufferTextureCount; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[i]);
        glTexStorage2D(GL_TEXTURE_2D, 1, m_GBufferTextureFormat[i], m_nWindowWidth, m_nWindowHeight);
    }

    glGenFramebuffers(1, &m_GBufferFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_GBufferFBO);
    for (int32_t i = GPosition; i < GDepth; ++i)
    {
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_GBufferTextures[i], 0);
    }
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_GBufferTextures[GDepth], 0);

    // we will write into 3 textures from the fragment shader
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, drawBuffers);

    GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "FB error, status: " << status << std::endl;
        throw std::runtime_error("FBO error");
    }

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);



    //Triangle recouvrant
    glGenBuffers(1, &m_TriangleVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_TriangleVBO);

    GLfloat data[] = { -1, -1, 3, -1, -1, 3 };
    glBufferStorage(GL_ARRAY_BUFFER, sizeof(data), data, 0);

    glGenVertexArrays(1, &m_TriangleVAO);
    glBindVertexArray(m_TriangleVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

        //Pour la vue FPS
        m_viewController.setViewMatrix(glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));

        //activer test profondeur GPU
        glEnable(GL_DEPTH_TEST);

}
