import { Sensor } from './Sensor';

export interface DataEntry {
    id: number,
    datetime: string,
    sensor: Sensor,
    value: number
}
