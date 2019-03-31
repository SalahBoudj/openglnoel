#pragma once
#include <iostream>
#include <vector>
#include <math.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glmlv/simple_geometry.hpp>
#include <glmlv/GLProgram.hpp>
#include <glmlv/ViewController.hpp>


	class Track {
		private:
		    float m_fDistance;
		    float m_fAngleX;
		    float m_fAngleY;

		public:
		    Track():
			m_fDistance(5.), m_fAngleX(0), m_fAngleY(0) {
		    }
		    //Permet d'avancer / reculer la caméra
		    void moveFront(float delta){
			    m_fDistance = m_fDistance + delta;
		    }
		    
	 	    //Permet de tourner latéralement autour du centre de vision
		    void rotateLeft(float degrees){
			    m_fAngleX = glm::radians(degrees);
		    }
		    
		    //Permet de tourner verticalement autour du centre de vision 
		    void rotateUp(float degrees){
			    m_fAngleY = glm::radians(degrees);
		    }
		    


		    //Calcul de la ViewMatrix de la caméra 
		    glm::mat4 getViewMatrix(float x, float z, float y) const{
		    
			glm::mat4 MatrixId = glm::mat4(1.0); //Matrice Identité


			glm::mat4 matrixMoveFront = glm::translate( MatrixId, glm::vec3(0, 0, -m_fDistance) ); //Translation en z
			glm::mat4 matrixrotateLeft = glm::rotate( MatrixId, m_fAngleX, glm::vec3(1, 0, 0) ); //Axe x
			glm::mat4 matrixrotateUp = glm::rotate( MatrixId, m_fAngleY, glm::vec3(0, 1, 0) ); //Axe y


			//translation pour être centré où l'on veut 
			glm::mat4 matrixMoveFront2 = glm::translate( MatrixId, glm::vec3(x, y, z) );

			return   matrixMoveFront * matrixrotateLeft * matrixrotateUp *  matrixMoveFront2;
		    
		    }

	};


