#include <QCoreApplication>
#include <QtBluetooth/qlowenergyadvertisingdata.h>
#include <QtBluetooth/qlowenergyadvertisingparameters.h>
#include <QtBluetooth/qlowenergycharacteristic.h>
#include <QtBluetooth/qlowenergycharacteristicdata.h>
#include <QtBluetooth/qlowenergydescriptordata.h>
#include <QtBluetooth/qlowenergycontroller.h>
#include <QtBluetooth/qlowenergyservice.h>
#include <QtBluetooth/qlowenergyservicedata.h>
#include <QtCore/qtimer.h>

#include <QtEndian>
#include <QFile>

uint32_t float754tofloat11073(float value)
{
    uint8_t  exponent = 0xFE; //exponent is -2
    uint32_t mantissa = (uint32_t)(value * 100);

    return (((uint32_t)exponent) << 24) | mantissa;
}

QByteArray readValueFromFile(QString filePath)
{
    QByteArray data;

    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly))
    {
        data = file.readAll();
        data.remove(data.length() - 1, 1);
        file.close();
    }

    return data;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QLowEnergyAdvertisingData advertisingData;                                              //serveur
    advertisingData.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityGeneral);  //Mode d'accessibilité
    advertisingData.setLocalName("OS #20 BLE");                                             //Nom du serveur
    advertisingData.setServices(QList<QBluetoothUuid>() <<
                                QBluetoothUuid::HeartRate <<
                                QBluetoothUuid::HealthThermometer <<
                                QBluetoothUuid::BloodPressure
                                );                         //Ajout des services prédéfinis

    //Création de la charactéristique
    QLowEnergyCharacteristicData charData;
    //spécification de la caractéristique voulue
    charData.setUuid(QBluetoothUuid::HeartRateMeasurement);
    charData.setValue(QByteArray(2, 0));
    //spécifique à la documentation bluetooth
    charData.setProperties(QLowEnergyCharacteristic::Notify);
    //Ajout d'un descripteur
    const QLowEnergyDescriptorData clientConfig(QBluetoothUuid::ClientCharacteristicConfiguration, QByteArray(2, 0));
    charData.addDescriptor(clientConfig);
    //Couplage du service avec la caractéristique créée.
    QLowEnergyServiceData serviceData;
    serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
    serviceData.setUuid(QBluetoothUuid::HeartRate);
    serviceData.addCharacteristic(charData);

    //Création de la charactéristique
    QLowEnergyCharacteristicData charTemperature;
    //spécification de la caractéristique voulue
    charTemperature.setUuid(QBluetoothUuid::TemperatureMeasurement);
    charTemperature.setValue(QByteArray(2, 0));
    //spécifique à la documentation bluetooth
    charTemperature.setProperties(QLowEnergyCharacteristic::Indicate);
    //Ajout d'un descripteur
    const QLowEnergyDescriptorData clientConfigTemp(QBluetoothUuid::ClientCharacteristicConfiguration,  QByteArray(2, 0));
    charTemperature.addDescriptor(clientConfigTemp);
    //Couplage du service avec la caractéristique créée.
    QLowEnergyServiceData serviceTemperatureData;
    serviceTemperatureData.setType(QLowEnergyServiceData::ServiceTypePrimary);
    serviceTemperatureData.setUuid(QBluetoothUuid::HealthThermometer);
    serviceTemperatureData.addCharacteristic(charTemperature);

    //Création de la charactéristique
    QLowEnergyCharacteristicData charPressure;
    //spécification de la caractéristique voulue
    charPressure.setUuid(QBluetoothUuid::BloodPressureMeasurement);
    charPressure.setValue(QByteArray(2, 0));
    //spécifique à la documentation bluetooth
    charPressure.setProperties(QLowEnergyCharacteristic::Indicate);
    //Ajout d'un descripteur
    const QLowEnergyDescriptorData clientConfigPressure(QBluetoothUuid::ClientCharacteristicConfiguration,  QByteArray(2, 0));
    charPressure.addDescriptor(clientConfigPressure);
    //Couplage du service avec la caractéristique créée.
    QLowEnergyServiceData servicePressureData;
    servicePressureData.setType(QLowEnergyServiceData::ServiceTypePrimary);
    servicePressureData.setUuid(QBluetoothUuid::BloodPressure);
    servicePressureData.addCharacteristic(charPressure);

    //advertise
    const QScopedPointer<QLowEnergyController> leController(QLowEnergyController::createPeripheral());
    const QScopedPointer<QLowEnergyService> service(leController->addService(serviceData));
    const QScopedPointer<QLowEnergyService> serviceTemperature(leController->addService(serviceTemperatureData));
    const QScopedPointer<QLowEnergyService> servicePressure(leController->addService(servicePressureData));

    leController->startAdvertising(QLowEnergyAdvertisingParameters(), advertisingData, advertisingData);

    /**
     * HEARTBEAT
     */
    QTimer heartbeatTimer;
    quint8 currentHeartRate = 60;
    enum ValueChange { ValueUp, ValueDown } valueChange = ValueUp;
    const auto heartbeatProvider = [&service, &currentHeartRate, &valueChange]() {
        QByteArray value;
        value.append(char(0)); // Flags that specify the format of the value.
        value.append(char(currentHeartRate)); // Actual value.
        QLowEnergyCharacteristic characteristic = service->characteristic(QBluetoothUuid::HeartRateMeasurement);
        Q_ASSERT(characteristic.isValid());
        service->writeCharacteristic(characteristic, value); // Potentially causes notification.
        if (currentHeartRate == 60)
            valueChange = ValueUp;
        else if (currentHeartRate == 100)
            valueChange = ValueDown;
        if (valueChange == ValueUp)
            ++currentHeartRate;
        else
            --currentHeartRate;
    };
    QObject::connect(&heartbeatTimer, &QTimer::timeout, heartbeatProvider);
    heartbeatTimer.start(1000);

    /**
     * TEMPERATURE
     */
    QTimer tempTimer;
    const auto tempProvider = [&serviceTemperature]() {

        QByteArray rawTemp = readValueFromFile("/sys/bus/iio/devices/iio\:device0/in_temp_raw");
        QByteArray scaleTemp = readValueFromFile("/sys/bus/iio/devices/iio\:device0/in_temp_scale");

        float temperature = rawTemp.toInt() * scaleTemp.toFloat();

        quint32 dataToSend = float754tofloat11073(temperature);

        QByteArray value;
        value.append(char(0)); // Flags that specify the format of the value.
        value.append(reinterpret_cast<const char*>(&dataToSend), sizeof(dataToSend)); // Actual value.

        QLowEnergyCharacteristic characteristic = serviceTemperature->characteristic(QBluetoothUuid::TemperatureMeasurement);
        Q_ASSERT(characteristic.isValid());
        serviceTemperature->writeCharacteristic(characteristic, value); // Potentially causes notification.
    };
    QObject::connect(&tempTimer, &QTimer::timeout, tempProvider);
    tempTimer.start(1000);


    /**
     * PRESSURE
     */
    QTimer pressureTimer;
    const auto pressureProvider = [&servicePressure]() {

        QByteArray rawPressure = readValueFromFile("/sys/bus/iio/devices/iio\:device0/in_pressure_raw");
        QByteArray scalePressure = readValueFromFile("/sys/bus/iio/devices/iio\:device0/in_pressure_scale");

        quint16 pressure = rawPressure.toInt() * scalePressure.toFloat();


        QByteArray value;
        value.append(char(1)); // Flags that specify the format of the value.
        value.append(reinterpret_cast<const char*>(&pressure), sizeof(pressure)); // Actual value.
        value.append(reinterpret_cast<const char*>(&pressure), sizeof(pressure)); // Actual value.
        value.append(reinterpret_cast<const char*>(&pressure), sizeof(pressure)); // Actual value.

        QLowEnergyCharacteristic characteristic = servicePressure->characteristic(QBluetoothUuid::BloodPressureMeasurement);
        Q_ASSERT(characteristic.isValid());
        servicePressure->writeCharacteristic(characteristic, value); // Potentially causes notification.
    };
    QObject::connect(&pressureTimer, &QTimer::timeout, pressureProvider);
    pressureTimer.start(1000);

    return a.exec();
}

