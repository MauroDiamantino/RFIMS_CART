/*
 * TestbenchReply.cpp
 *
 *  Created on: 04/03/2019
 *      Author: new-mauro
 */
#include "SpectranInterface.h"


int main()
{
	Reply reply(RBW_INDEX);
	vector<uint8_t> bytesVector;

	//Verificacion de la respuesta VERIFY
	reply.PrepareTo(Reply::VERIFY);
	assert(reply.GetReplyType()==Reply::VERIFY);
	assert(reply.GetNumOfBytes()==5);
	bytesVector.push_back(0x01);
	bytesVector.push_back(0x51);
	bytesVector.push_back(0x1A);
	bytesVector.push_back(0xF5);
	bytesVector.push_back(0xAF);
	reply.InsertBytes( bytesVector.data() );
	assert(reply.IsRightReply()==true);

	//Verificacion del metodo Clear()
	bytesVector.clear();
	reply.Clear();
	assert(reply.GetReplyType()==Reply::UNINITIALIZED);
	assert(reply.GetNumOfBytes()==0);
	assert(reply.GetValue()==0.0);

	//Verificacion de las respuestas GETSTPVAR al consultar las variables STARTFREQ y STOPFREQ, considerando que ambas
	//estan configuradas en 1 MHz. Se debe tener en cuenta que el valor enviado por el analizador de espectro estara
	//en MHz, es decir devolvera 1.0, pero el objeto debe convertirlo a Hz, por eso se evaluara que el valor que entrega
	//el objeto sea 1.0e6.
	for (auto i=1; i<=2; i++){
		reply.PrepareTo(Reply::GETSTPVAR, VarName(i) );
		assert(reply.GetReplyType()==Reply::GETSTPVAR);
		assert(reply.GetNumOfBytes()==6);
		bytesVector.push_back(0x20);
		bytesVector.push_back(0);
		bytesVector.push_back(0x00); //Byte menos significativo del valor flotante 1.0
		bytesVector.push_back(0x00);
		bytesVector.push_back(0x80);
		bytesVector.push_back(0x3F); //Byte mas significativo del valor flotante 1.0
		reply.InsertBytes( bytesVector.data() );
		assert(reply.IsRightReply()==true);
		assert(reply.GetValue()==1e6); //El valor se entrega en Hz en vez de MHz
		bytesVector.clear();
		reply.Clear();
	}

	//Verificacion de las respuestas GETSTPVAR al consultar las variables RESBANDW y VIDBANDW, considerando que ambas
	//estan configuradas en 200 Hz. Se debe tener en cuenta que el valor entregado por el analizador de espectro es el
	//indice RBW correspondiente al valor configurado, en este caso sera 102.0, por lo tanto se evaluara si el objeto
	//lo convierte automaticamente al valor en Hz.
	for (auto i=3; i<=4; i++){
		reply.PrepareTo(Reply::GETSTPVAR, VarName(i) );
		assert(reply.GetReplyType()==Reply::GETSTPVAR);
		assert(reply.GetNumOfBytes()==6);
		bytesVector.push_back(0x20);
		bytesVector.push_back(0);
		bytesVector.push_back(0x00); //Byte menos significativo del valor flotante 102.0
		bytesVector.push_back(0x00);
		bytesVector.push_back(0xCC);
		bytesVector.push_back(0x42); //Byte mas significativo del valor flotante 102.0
		reply.InsertBytes( bytesVector.data() );
		assert(reply.IsRightReply()==true);
		assert(reply.GetValue()==200.0); //El valor se entrega en Hz en vez de entregar el indice correspondiente
		bytesVector.clear();
		reply.Clear();
	}

	//Verificacion de las respuestas GETSTPVAR sobre las variables 0x09 a 0x0B. En este caso el analizador de espectro
	//entrega el mismo valor con el que esta configurada la variable, por lo que se evaluara que el objeto no lo
	//convierta. Ademas se evaluara el constructor mas completo de la clase y el metodo GetReplyTypeString().
	//Se considera que todas las variables estan configuradas en 0.0
	for (auto i=0x09; i<=0x0B; i++){
		Reply reply2(RBW_INDEX, Reply::GETSTPVAR, VarName(i));
		assert(reply2.GetReplyTypeString()=="GETSTPVAR");
		assert(reply2.GetNumOfBytes()==6);
		bytesVector.push_back(0x20);
		bytesVector.push_back(0);
		bytesVector.insert(bytesVector.begin()+2, 4, 0); //Valor flotante 0.0
		reply2.InsertBytes( bytesVector.data() );
		assert(reply2.IsRightReply()==true);
		assert(reply2.GetValue()==0.0);
		bytesVector.clear();
	}

	//Verificacion de la respuesta a los comandos SETSTPVAR. Estas respuestas solo tienen 2 bytes: el primero las identifica
	//(0x21) y el siguiente byte indica si el estado de la operacion (exito==0.0 y error!=0.0). No brindan ningun tipo de
	//informacion respecto de la variable que se modifico
	reply.PrepareTo(Reply::SETSTPVAR);
	assert(reply.GetReplyType()==Reply::SETSTPVAR);
	assert(reply.GetNumOfBytes()==2);
	bytesVector.push_back(0x21);
	bytesVector.push_back(0);
	reply.InsertBytes( bytesVector.data() );
	assert(reply.IsRightReply()==true);
	bytesVector.clear();
	reply.Clear();

	//Verificacion de la respuesta AMPFREQDAT. Esta respuesta esta formada por 17 bytes: un header, 4 bytes de timestamp
	//(valor entero), 4 bytes para la frecuencia (valor flotante en Hz/10), 4 bytes para la potencia minima
	//(valor flotante, en dBm) y 4 bytes para la potencia maxima (valor flotante, en dBm). Se considerara que el
	//timestamp es 1000, la frecuencia 850.0 MHz, la potencia minima -50.2 dBm y la potencia maxima -45.5 dBm.
	//Se debe tener en cuenta que con objetos de la clase base Reply solo se puede obtener el valor de potencia minima
	//con esta clase de respuesta
	reply.PrepareTo(Reply::AMPFREQDAT);
	assert(reply.GetReplyType()==Reply::AMPFREQDAT);
	assert(reply.GetNumOfBytes()==17);
	bytesVector.push_back(0x22); //header de la respuesta
	bytesVector.push_back(0xE8); //Byte menos significativo del timpestamp
	bytesVector.push_back(0x03);
	bytesVector.push_back(0x00);
	bytesVector.push_back(0x00); //Byte mas significativo del timestamp
	bytesVector.push_back(0xE8); //Byte menos significativo de la frecuencia
	bytesVector.push_back(0x1F);
	bytesVector.push_back(0xA2);
	bytesVector.push_back(0x4C); //Byte mas significativo de la frecuencia
	bytesVector.push_back(0xCD); //Byte menos significativo de la potencia minima
	bytesVector.push_back(0xCC);
	bytesVector.push_back(0x48);
	bytesVector.push_back(0xC2); //Byte mas significativo de la potencia minima
	bytesVector.push_back(0x00); //Byte menos significativo de la potencia maxima
	bytesVector.push_back(0x00);
	bytesVector.push_back(0x36);
	bytesVector.push_back(0xC2); //Byte mas significativo de la potencia maxima
	reply.InsertBytes( bytesVector.data() );
	assert(reply.IsRightReply()==true);
	assert(reply.GetValue()==-50.2f);


	//Verificacion de la respuesta AMPFREQDAT pero ahora usando un objecto de la clase derivada SweepReply, que esta
	//diseñada especificamente para manejar este tipo de respuesta. Ahora si sera posible acceder al timestamp, frecuencia
	//valor minimio de potencia y valor maximo. De nuevo se considerara que el timestamp es 1000, la frecuencia es
	//850.0 MHz, la potencia minima es -50.2 dBm y la potencia maxima es -45.5 dBm.
	SweepReply swReply;
	assert(swReply.GetReplyType()==Reply::AMPFREQDAT);
	assert(swReply.GetNumOfBytes()==17);
	//El vector de bytes no fue borrado por lo que no es necesario llenarlo de nuevo
	swReply.InsertBytes( bytesVector.data() );
	assert(swReply.IsRightReply()==true);
	assert(swReply.GetTimestamp()==1000);
	assert(swReply.GetFrequency()==850e6);
	assert(swReply.GetMinValue()==-50.2f);
	assert(swReply.GetValue()==-50.2f);
	assert(swReply.GetMaxValue()==-45.5);

	SweepReply swReply2;
	swReply2 = swReply;
	assert(swReply2.GetValue()==-50.2f);

	cout << "El testbench de la clase Reply finalizo con resultados exitosos. Sigue asi!!" << endl;
}
