import { Sensor } from './Sensor';

export interface DataEntry {
    datetime: string,
    sensor: Sensor,
    value: number
}
