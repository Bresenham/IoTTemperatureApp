export enum SensorType {
    Temperature = "Temperature"
}

export interface Sensor {
    id: number,
    type: SensorType
}
