/*! \file AntennaPositioner.cpp
 * 	\brief This file contains the definitions of several methods of the class _AntennaPositioner_.
 * 	\author Emanuel Asencio
 */

#include "AntennaPositioning.h"


void AntennaPositioner::inicia_variables()
{
#ifdef RASPBERRY_PI
	//SE INICIAN LAS INTERRUPCIONES
	wiringPiISR(FASE_A, INT_EDGE_RISING, canalA);
	wiringPiISR(FASE_B, INT_EDGE_RISING, canalB);
#endif
}

/*! \param gpsInterf A reference to the object which is responsible for the communication with the Aaronia GPS receiver. */
AntennaPositioner::AntennaPositioner(GPSInterface & gpsInterf) : gpsInterface(gpsInterf)
{
	pos_actual = {0.0, 0.0, 0};
	pos_anterior = {0.0, 0.0, 0};
	cuenta = 0.0;
	yaw = 0.0;
	//roll = 0.0;
	n = 2.0;
	cantPosiciones = numOfAzimPos; //Este valor es recibido como un argumento del software que se guarda en la variable 'numOfAzimPos'
	polar = 0; //POLAR = 0 (HORIZONTAL) ; POLAR = 1 (VERTICAL)
}

/*!	This initialization implies to move the antenna to its initial position, to capture the initial azimuth
 * angle and to ensure the antenna polarization is horizontal.
 * \return A `true` if the initialization was successful or a `false` otherwise.
 */
bool AntennaPositioner::Initialize()
{
#ifdef RASPBERRY_PI
	//SE PONE EN BAJO LA HABILITACION
	digitalWrite(EN,LOW);
	// PONER LA ANTENA EN POSICION HORIZONTAL
	digitalWrite(POL,LOW); // PONE LA ANTENA EN POSICION HORIZONTAL
#endif

	double aux = 100.0;
	while ( (aux < -5.0) || (aux > 5.0) )
	{
		gpsInterface.UpdateRoll();
		aux = gpsInterface.GetRoll();//rotacion
	}
	polar = 0;// AHORA LA ANTENA QUEDO EN POLARIZACION HORIZONTAL

#ifdef RASPBERRY_PI
	//SE PONE EN ALTO LA HABILITACION
	digitalWrite(EN,HIGH);

	//SE EMPIEZA EN UNA DIRECCION(DIR,HIGH) (IZQUIERDA) sentido anti-horario
	digitalWrite(DIRECCION,HIGH);

	cuenta=0;
	while( (cuenta/n) > -360 && cuenta <= 0 )
	{
		if (0 == digitalRead(SENSOR_NORTE)) //A TRAVES DE UNA SE�AL (0 O 1) SE VERA DONDE ESTA EL NORTE
		{
			if(false == poneEnCero())
				return(false);
			return(true);
		}
		un_paso();
	}

	//SE PONE EN BAJO LA HABILITACION
	digitalWrite(EN,LOW);
#endif
	return(false);
}

/*!	To move the antenna to the next position, this is rotated an angle determined by the number of positions
 * (360/number of positions) and in a clockwise way (seen from above). If the current position is the last one,
 * then the antenna is move to the initial position, rotating this one counterclockwise (seen from above) to
 * avoid the cables to tangle or stretch.
 *
 * Taking into account the method Initialize(), it is waited this method to be called when the antenna polarization
 * changes from vertical to horizontal.
 * \return A `true` if the operation was successful or a `false` otherwise.
 */
bool AntennaPositioner::NextAzimPosition()
{
	float pasos_aux;

#ifdef RASPBERRY_PI
	//SE PONE EN ALTO LA HABILITACION
	digitalWrite(EN,HIGH);
	digitalWrite(DIRECCION,LOW);//DERECHA ---> ME MUEVO EN SENTIDO CONTRARIO POR LOS CABLES sentido horario
#endif

	//GUARDA LOS DATOS ACTUALES A LOS DATOS ANTERIORES
	if(false == guardaDatos())
	{
#ifdef RESPBERRY_PI
		digitalWrite(EN,LOW);
#endif
		return(false);
	}

	if ((pos_actual.pasos == pos_anterior.pasos) && (pos_actual.posicion != cantPosiciones))
	{
		pasos_aux = mover();
		if(false == actualizaActual(pasos_aux))
		{
#ifdef RASPBERRY_PI
			digitalWrite(EN,LOW);
#endif
			return(false);
		}
	}
	else if ((pos_actual.pasos == pos_anterior.pasos) && (pos_actual.posicion == cantPosiciones))
	{
		if(false == poneEnCero())
		{
#ifdef RASPBERRY_PI
			digitalWrite(EN,LOW);
#endif
			return(false);
		}
			//regresar(cantPosiciones);
		if(false == regresar())
		{
#ifdef RASPBERRY_PI
			digitalWrite(EN,LOW);
#endif
			return(false);
		}
	}

#ifdef RASPBERRY_PI
	//SE PONE EN BAJO LA HABILITACION
	digitalWrite(EN,LOW);
#endif

	return(true);
}

/*! If the current polarization is horizontal, then it is changed to vertical, and vice versa.
 * \return A `true` if the operation was successful or a `false` otherwise.
 */
bool AntennaPositioner::ChangePolarization()
{
#ifdef RASPBERRY_PI
	//SE PONE EN ALTO LA HABILITACION
	digitalWrite(EN,LOW);
#endif

	// INVERTIR LA POLARIZACION DE LA FUENTE DE ALIMENTACION DEL MOTOR QUE MANEJA LA POLARIZACION
	float aux=0;
	 // COMO ESTA EN POLARIZACION HORIZONTAL LA CAMBIA A VERTICAL
    if (polar == 0)
    {
#ifdef RASPBERRY_PI
        digitalWrite(POL,HIGH); // PONE LA ANTENA EN POSICION VERTICAL
#endif
        aux = 100;
        while ( ((aux < 85) || (aux > 95)) )
        {
            gpsInterface.UpdateRoll();
            aux = gpsInterface.GetRoll();//rotacion
        }
        polar = 1; // AHORA LA ANTENA QUEDO EN POLARIZACION VERTICAL
        return(true);
    }
    else
    {
        aux=100;
#ifdef RASPBERRY_PI
        digitalWrite(POL,LOW); // PONE LA ANTENA EN POSICION HORIZONTAL
#endif
        while ( ((aux < -5) || (aux > 5)) )
        {
            gpsInterface.UpdateRoll();
            aux = gpsInterface.GetRoll();//rotacion
        }
        polar = 0;// AHORA LA ANTENA QUEDO EN POLARIZACION HORIZONTAL
        return(true);
    }
}

/*! \return A `std::string` object with the current antenna polarization.	*/
std::string AntennaPositioner::GetPolarizationString() const
{
	switch(polar)
	{
	case 0:
		return "horizontal";
	case 1:
		return "vertical";
	default:
		return "unknown";
	}
}

//DA UN PASO
void AntennaPositioner::un_paso()
{
#ifdef RASPBERRY_PI
	const unsigned int RETARDO = 5;//MILISEGUNDOS
    digitalWrite(PUL,HIGH); // SE ENVIA UN PULSO EN ALTO(PUL,HIGH)
    delay(RETARDO);  // SE ESPERA UN TIEMPO(RETARDO)
    digitalWrite(PUL,LOW); // SE ENVIA UN PULSO EN BAJO(PUL,LOW)
    delay(RETARDO);
#endif
}

//MUEVE X GRADOS Y RETORNA LOS PASOS DADOS
float AntennaPositioner::mover()
{
    cuenta=0;
    float aux = ( 360.0 / cantPosiciones );
    while(((cuenta/n) < aux) && (cuenta >= 0)) {
        un_paso();
    }
    return((cuenta/n));
}

//ACTUALIZA LOS DATOS ACTUALES
bool AntennaPositioner::actualizaActual(float pasos_aux)
{
    if((pasos_aux > ((360.0 / cantPosiciones)-2)) && (pasos_aux < ((360.0 / cantPosiciones)+2)))
        pos_actual.posicion=pos_actual.posicion + 1;

    pos_actual.grados=pos_actual.grados + (pasos_aux);
    pos_actual.pasos=pos_actual.pasos + pasos_aux;

    return(true);
}

//RECIBE LOS DATOS ACTUALES Y ANTERIORES Y LOS GUARDA EN EL ANTERIOR
bool AntennaPositioner::guardaDatos()
{

    pos_anterior.posicion=pos_actual.posicion;
    pos_anterior.grados=pos_actual.grados;
    pos_anterior.pasos=pos_actual.pasos;

    return(true);
}

// Pone en cero todos los datos de posicion, pasos y grados
bool AntennaPositioner::poneEnCero()
{

    pos_actual.grados=0;pos_actual.pasos=0;pos_actual.posicion=0;
    pos_anterior.grados=0;pos_anterior.pasos=0;pos_anterior.posicion=0;

    return(true);
}

bool AntennaPositioner::regresar()
{
#ifdef RASPBERRY_PI
	digitalWrite(DIRECCION,HIGH);// REGRESO POR LA IZQUIERDA ANTI HORARIO
#endif
	cuenta=0;
	float aux = -((360.0 / cantPosiciones) * (cantPosiciones));
    while(((cuenta/n) >= aux) && (cuenta<=0))
    {
        un_paso();
            if( ( (cuenta/n) > (aux-1) && (cuenta/n) < (aux+2) ) )
                return true;
    }
    return(false);
}

//#/////////////////////////////INTERRUPCIONES//////////////////////////

void canalA()
{
#ifdef RASPBERRY_PI
    if(digitalRead(FASE_B) == 0){
        cuenta++;
    }else{
        cuenta--;
    }
#endif
}

void canalB()
{
#ifdef RASPBERRY_PI
    if(digitalRead(FASE_A) == 0){
        cuenta--;
    }else{
        cuenta++;
    }
#endif
}
