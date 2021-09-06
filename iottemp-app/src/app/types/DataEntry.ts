import { Sensor } from './Sensor';

export interface DataEntry {
    datetime: Date,
    sensor: Sensor,
    value: number
}
