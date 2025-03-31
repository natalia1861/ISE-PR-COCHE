/**
 ******************************************************************************
 * @file           : nak_Driver_I2C.c
 * @author         : Natalia Agüero
 * @date           : 19/10/2024
 * @brief          : Driver I2C
 ******************************************************************************
 * @attention
 * 
 * Este archivo contiene la implementación del driver I2C,
 * 
 ******************************************************************************
 */
 
#include "nak_Driver_I2C.h"
#include <stdio.h>
#include "RTE_Device.h"
#include "stm32f4xx.h"       // Cabecera de CMSIS para acceso al NVIC y otros periféricos


//definition of I2C depending if its active or not

 ARM_I2C_STATUS estado;
 uint32_t state;
 
I2C_DriverConfig_t Drivers_I2C [I2C_LINE_MAX] = {
    [I2C_LINE_1] = {
        .name = "I2C1",
        .initialized = false
    },
    [I2C_LINE_2] = {
        .name = "I2C2",
        .initialized = false
    },
    [I2C_LINE_3] = {
        .name = "I2C3",
        .initialized = false
    }
};

#if defined (RTE_I2C1) && (RTE_I2C1 == 1)
    #define USE_I2C1
    extern ARM_DRIVER_I2C Driver_I2C1; 
    void I2C1_callback(uint32_t event);
#endif

#if defined (RTE_I2C2) && (RTE_I2C2 == 1)
    #define USE_I2C2
    extern ARM_DRIVER_I2C Driver_I2C2;
    static void I2C2_callback(uint32_t event);

#endif

#if defined (RTE_I2C3) && (RTE_I2C3 == 1)
    #define USE_I2C3
    extern ARM_DRIVER_I2C Driver_I2C3;
    static void I2C3_callback(uint32_t event);

#endif


int32_t I2C_Init_All (void) {
    int32_t status = ARM_DRIVER_OK;  // Inicialización del estado
	#ifdef USE_I2C1
            Drivers_I2C[I2C_LINE_1].driver = &Driver_I2C1;  // Asigna el driver para I2C1
            Drivers_I2C[I2C_LINE_1].callback_I2C = &I2C1_callback; // Callback para I2C1
            Drivers_I2C[I2C_LINE_1].initialized = false;  // Marca que no está inicializado
			status |= I2C_Init(I2C_LINE_1);  // Inicializa I2C1
	#endif
	#ifdef USE_I2C2
            Drivers_I2C[I2C_LINE_2].driver = &Driver_I2C2;  // Asigna el driver para I2C2
            Drivers_I2C[I2C_LINE_2].callback_I2C = I2C2_callback; // Callback para I2C2
            Drivers_I2C[I2C_LINE_2].initialized = false;  // Marca que no está inicializado
			status |= I2C_Init(I2C_LINE_2);  // Inicializa I2C2
	#endif
	#ifdef USE_I2C3
            Drivers_I2C[I2C_LINE_3].driver = &Driver_I2C3;  // Asigna el driver para I2C3
            Drivers_I2C[I2C_LINE_3].callback_I2C = I2C3_callback; // Callback para I2C3
            Drivers_I2C[I2C_LINE_3].initialized = false;  // Marca que no está inicializado
			status |= I2C_Init(I2C_LINE_3);  // Inicializa I2C3
	#endif	
    return status;  // Retorna el estado de la inicialización
}


int32_t I2C_Init(I2C_LINE I2C_line) {
	// Verifica si la línea ya está inicializada
		if (Drivers_I2C[I2C_line].initialized) {
			return ARM_DRIVER_OK;  // Si ya está inicializado, retorna OK
		}
		
		int32_t status = ARM_DRIVER_OK;  // Estado inicial

        // Configuración del mutex para el acceso exclusivo al bus I2C
		char name[20];
		sprintf(name, "%s%s", "mutex ", Drivers_I2C[I2C_line].name);
		const osMutexAttr_t i2c_mutex_attr = {
			name  // Asigna el nombre al mutex
		};
		
		// Crea el mutex para controlar el acceso al bus I2C
        if ((Drivers_I2C[I2C_line].mutex_I2C = osMutexNew(&i2c_mutex_attr)) == NULL) {
			return ARM_DRIVER_ERROR;  // Error al crear el mutex
		}			

        // Crea el semáforo para controlar las transferencias
        if ((Drivers_I2C[I2C_line].transfer_I2C_semaphore = osSemaphoreNew(1, 1, NULL)) == NULL) {  // Inicialmente bloqueado
            return ARM_DRIVER_ERROR;  // Error al crear el semáforo
		}
		
        // Inicializa el controlador I2C
        status |= Drivers_I2C[I2C_line].driver->Initialize(Drivers_I2C[I2C_line].callback_I2C);
        status |= Drivers_I2C[I2C_line].driver->PowerControl(ARM_POWER_FULL);
        status |= Drivers_I2C[I2C_line].driver->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);  // Usar velocidad rápida (400kHz) por defecto
        
        // Verifica si la inicialización fue exitosa
        estado = Drivers_I2C[I2C_line].driver->GetStatus();
        
        if (status == ARM_DRIVER_OK) {
            Drivers_I2C[I2C_line].initialized = true;  // Marca la línea como inicializada
		}
    return status;  // Retorna el estado de la inicialización
}

int32_t I2C_Configure(I2C_LINE I2C_line, I2C_configuration_t configuration) {
    uint32_t status = ARM_DRIVER_OK;

    // Inicializa el controlador I2C
    status |= Drivers_I2C[I2C_line].driver->Initialize(Drivers_I2C[I2C_line].callback_I2C);
    status |= Drivers_I2C[I2C_line].driver->PowerControl(ARM_POWER_FULL);
    status |= Drivers_I2C[I2C_line].driver->Control(ARM_I2C_BUS_SPEED, configuration.speed);  // Usar velocidad rápida (400kHz) por defecto
    status |= Drivers_I2C[I2C_line].driver->Control(ARM_I2C_BUS_CLEAR, 0);

    return status;  // El dispositivo está listo
}

int32_t I2C_TestSensor (I2C_LINE I2C_line, uint32_t slave_address) {
    
    // Adquiere el mutex antes de acceder al bus I2C
	if (osMutexAcquire(Drivers_I2C[I2C_line].mutex_I2C, osWaitForever) != osOK)
    {
        return ARM_DRIVER_ERROR;
    }
    
	//Se adquiere el semaforo de transferencias
	if (osSemaphoreAcquire(Drivers_I2C[I2C_line].transfer_I2C_semaphore, osWaitForever) != osOK)
    {
        osMutexRelease(Drivers_I2C[I2C_line].mutex_I2C);
        return ARM_DRIVER_ERROR;  // Error en la transmisión
    }
    
     // Master command
    if (Drivers_I2C[I2C_line].driver->MasterTransmit(slave_address, NULL, 0, false) != ARM_DRIVER_OK)
    {
        osMutexRelease(Drivers_I2C[I2C_line].mutex_I2C);
        return ARM_DRIVER_ERROR;  // Error en la transmisión
    }

	//Se espera a que la transferencia se complete
    if (osSemaphoreAcquire(Drivers_I2C[I2C_line].transfer_I2C_semaphore, osWaitForever) != osOK)
    {
        osMutexRelease(Drivers_I2C[I2C_line].mutex_I2C);
        return ARM_DRIVER_ERROR;  // Error en la transmisión
    }
    
    //Slave response
	if (Drivers_I2C[I2C_line].driver->MasterReceive(slave_address, NULL, 0, false) != ARM_DRIVER_OK)
    {
        osMutexRelease(Drivers_I2C[I2C_line].mutex_I2C);
        return ARM_DRIVER_ERROR;  // Error en la recepción
    }
    
    // Liberar el mutex después de usar el I2C
    osMutexRelease(Drivers_I2C[I2C_line].mutex_I2C);  
    
    return ARM_DRIVER_OK;
}

#ifdef USE_I2C1
void I2C1_callback(uint32_t event) {
    uint8_t status = 0;
	if (event & ARM_I2C_EVENT_TRANSFER_DONE)
    {
        if (osSemaphoreRelease(Drivers_I2C[I2C_LINE_1].transfer_I2C_semaphore) != osOK)
        {
            status = 5;
        }
    
    } else {
          // Manejo de errores específicos
        if (event & ARM_I2C_EVENT_TRANSFER_INCOMPLETE) {
            osSemaphoreRelease(Drivers_I2C[I2C_LINE_1].transfer_I2C_semaphore);
            status = 1;
        }
        if (event & ARM_I2C_EVENT_ARBITRATION_LOST) {
            osSemaphoreRelease(Drivers_I2C[I2C_LINE_1].transfer_I2C_semaphore);
            status = 2;
        }
        if (event & ARM_I2C_EVENT_BUS_ERROR) {
            osSemaphoreRelease(Drivers_I2C[I2C_LINE_1].transfer_I2C_semaphore);
            status = 3;
        }
        if (event & ARM_I2C_EVENT_BUS_CLEAR) {
            osSemaphoreRelease(Drivers_I2C[I2C_LINE_1].transfer_I2C_semaphore);
            status = 4;
        }
	}
}
#endif
#ifdef USE_I2C2
void I2C2_callback(uint32_t event) {
    uint8_t status = 0;
	if (event & ARM_I2C_EVENT_TRANSFER_DONE)
    {
        if (osSemaphoreRelease(Drivers_I2C[I2C_LINE_2].transfer_I2C_semaphore) != osOK)
        {
            status = 5;
        }
    
    } else {
          // Manejo de errores específicos
        if (event & ARM_I2C_EVENT_TRANSFER_INCOMPLETE) {
            osSemaphoreRelease(Drivers_I2C[I2C_LINE_2].transfer_I2C_semaphore);
            status = 1;
        }
        if (event & ARM_I2C_EVENT_ARBITRATION_LOST) {
            osSemaphoreRelease(Drivers_I2C[I2C_LINE_2].transfer_I2C_semaphore);
            status = 2;
        }
        if (event & ARM_I2C_EVENT_BUS_ERROR) {
            osSemaphoreRelease(Drivers_I2C[I2C_LINE_2].transfer_I2C_semaphore);
            status = 3;
        }
        if (event & ARM_I2C_EVENT_BUS_CLEAR) {
            osSemaphoreRelease(Drivers_I2C[I2C_LINE_2].transfer_I2C_semaphore);
            status = 4;
        }
	}
}
#endif
#ifdef USE_I2C3
void I2C3_callback(uint32_t event) {
    uint8_t status = 0;
	if (event & ARM_I2C_EVENT_TRANSFER_DONE)
    {
        if (osSemaphoreRelease(Drivers_I2C[I2C_LINE_3].transfer_I2C_semaphore) != osOK)
        {
            status = 5;
        }
    
    } else {
          // Manejo de errores específicos
        if (event & ARM_I2C_EVENT_TRANSFER_INCOMPLETE) {
            osSemaphoreRelease(Drivers_I2C[I2C_LINE_3].transfer_I2C_semaphore);
            status = 1;
        }
        if (event & ARM_I2C_EVENT_ARBITRATION_LOST) {
            osSemaphoreRelease(Drivers_I2C[I2C_LINE_3].transfer_I2C_semaphore);
            status = 2;
        }
        if (event & ARM_I2C_EVENT_BUS_ERROR) {
            osSemaphoreRelease(Drivers_I2C[I2C_LINE_3].transfer_I2C_semaphore);
            status = 3;
        }
        if (event & ARM_I2C_EVENT_BUS_CLEAR) {
            osSemaphoreRelease(Drivers_I2C[I2C_LINE_3].transfer_I2C_semaphore);
            status = 4;
        }
	}
}
#endif

int32_t I2C_ReadRegister (I2C_LINE I2C_line, uint32_t SLAVE_ADDRESS, uint8_t reg, uint8_t* data) {
    
    // Adquirir el mutex antes de acceder al I2C
    if (osMutexAcquire(Drivers_I2C[I2C_line].mutex_I2C, osWaitForever) != osOK)
    {
        return ARM_DRIVER_ERROR;
    }
    
	//Adquiere el semaforo de transferencias
	if (osSemaphoreAcquire(Drivers_I2C[I2C_line].transfer_I2C_semaphore, osWaitForever) != osOK)
    {
        osSemaphoreRelease(Drivers_I2C[I2C_LINE_1].transfer_I2C_semaphore);
        osMutexRelease(Drivers_I2C[I2C_line].mutex_I2C);
        return ARM_DRIVER_ERROR;
    }
	
    // Master command
    if (Drivers_I2C[I2C_line].driver->MasterTransmit(SLAVE_ADDRESS, &reg, 1, false) != ARM_DRIVER_OK) 
    {
        osSemaphoreRelease(Drivers_I2C[I2C_LINE_1].transfer_I2C_semaphore);
        osMutexRelease(Drivers_I2C[I2C_line].mutex_I2C);
        return ARM_DRIVER_ERROR;  // Error en la transmisión
    }
    
    //Se espera a que la transferencia se complete
    if (osSemaphoreAcquire(Drivers_I2C[I2C_line].transfer_I2C_semaphore, osWaitForever) != osOK)
    {
        osMutexRelease(Drivers_I2C[I2C_line].mutex_I2C);
        return ARM_DRIVER_ERROR;
    }

    //Slave response
    if (Drivers_I2C[I2C_line].driver->MasterReceive(SLAVE_ADDRESS, data, 1, false) != ARM_DRIVER_OK)
    {
        osSemaphoreRelease(Drivers_I2C[I2C_LINE_1].transfer_I2C_semaphore);
        osMutexRelease(Drivers_I2C[I2C_line].mutex_I2C);
        return ARM_DRIVER_ERROR;
    }
   
    osMutexRelease(Drivers_I2C[I2C_line].mutex_I2C);

    estado = Drivers_I2C[I2C_line].driver->GetStatus();
    
    return ARM_DRIVER_OK;
}


int32_t I2C_ReadRegisters (I2C_LINE I2C_line, uint32_t SLAVE_ADDRESS, uint8_t reg, uint8_t* data, uint8_t size) {
  uint32_t status = ARM_DRIVER_OK;
  estado = Drivers_I2C[I2C_line].driver->GetStatus();
    // Adquirir el mutex antes de acceder al I2C
	if (osMutexAcquire(Drivers_I2C[I2C_line].mutex_I2C, osWaitForever) != osOK)
    {
        return ARM_DRIVER_ERROR;  // Error en la transmisión
    }
	//adquiere el semaforo de transferencias
    if (osSemaphoreAcquire(Drivers_I2C[I2C_line].transfer_I2C_semaphore, osWaitForever) != osOK)
    {
        osMutexRelease(Drivers_I2C[I2C_line].mutex_I2C);
        return ARM_DRIVER_ERROR;  // Error en la transmisión
    }
	
  // Master command
    if ((state = Drivers_I2C[I2C_line].driver->MasterTransmit(SLAVE_ADDRESS, &reg, 1, false)) != osOK)
    {
        osSemaphoreRelease(Drivers_I2C[I2C_LINE_1].transfer_I2C_semaphore);
        osMutexRelease(Drivers_I2C[I2C_line].mutex_I2C);
        return ARM_DRIVER_ERROR;  // Error en la transmisión
    }
    
    //Se espera a que la transferencia se complete
    if (osSemaphoreAcquire(Drivers_I2C[I2C_line].transfer_I2C_semaphore, osWaitForever) != osOK)
    {
        osMutexRelease(Drivers_I2C[I2C_line].mutex_I2C);
        return ARM_DRIVER_ERROR;  // Error en la transmisión
    }

	//Slave response
	if (Drivers_I2C[I2C_line].driver->MasterReceive(SLAVE_ADDRESS, data, size, false) != ARM_DRIVER_OK)
    {
        osSemaphoreRelease(Drivers_I2C[I2C_LINE_1].transfer_I2C_semaphore);
        osMutexRelease(Drivers_I2C[I2C_line].mutex_I2C);
        return ARM_DRIVER_ERROR;  // Error en la recepción
    }
    
    osMutexRelease(Drivers_I2C[I2C_line].mutex_I2C);  // Liberar el mutex después de usar el I2C

	estado = Drivers_I2C[I2C_line].driver->GetStatus();
    return ARM_DRIVER_OK;
}

int32_t I2C_WriteRegister (I2C_LINE I2C_line, uint32_t SLAVE_ADDRESS, uint8_t reg, uint8_t data) {
	uint8_t aux [2] = {reg, data};
    
	// Adquirir el mutex antes de acceder al I2C
	if (osMutexAcquire(Drivers_I2C[I2C_line].mutex_I2C, osWaitForever) != osOK)
    {
        return ARM_DRIVER_ERROR;
    }
	//adquiere el semaforo de transferencias
    if (osSemaphoreAcquire(Drivers_I2C[I2C_line].transfer_I2C_semaphore, osWaitForever) != osOK)
    {
        osMutexRelease(Drivers_I2C[I2C_line].mutex_I2C);
        return ARM_DRIVER_ERROR;  // Error en la transmisión
    }
	//Master command
    if (Drivers_I2C[I2C_line].driver->MasterTransmit(SLAVE_ADDRESS, aux, 2, false) != ARM_DRIVER_OK) 
    {
        osSemaphoreRelease(Drivers_I2C[I2C_LINE_1].transfer_I2C_semaphore);
        osMutexRelease(Drivers_I2C[I2C_line].mutex_I2C);
        return ARM_DRIVER_ERROR;  // Error en la transmisión
    }
    
    osMutexRelease(Drivers_I2C[I2C_line].mutex_I2C);  // Liberar el mutex después de usar el I2C

    estado = Drivers_I2C[I2C_line].driver->GetStatus();
    return ARM_DRIVER_OK;
}
