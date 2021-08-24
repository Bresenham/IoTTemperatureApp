import { Injectable } from '@angular/core';
import { formatDate } from '@angular/common';

import { DataEntry } from '../types/DataEntry';
import { SensorType } from '../types/Sensor';

const Entries : DataEntry[] = [];

@Injectable({
  providedIn: 'root'
})
export class DataService {

  constructor() {
    Entries.push(
      {
        id: 1,
        datetime: formatDate(Date.now(), 'dd/MM/yyyy HH:mm:ss', 'en-US'),
        sensor: {
          id: 573015,
          type: SensorType.Temperature
        },
        value: 2502
      },
      {
        id: 2,
        datetime: formatDate(Date.now(), 'dd/MM/yyyy HH:mm:ss', 'en-US'),
        sensor: {
          id: 219581,
          type: SensorType.Temperature
        },
        value: 2752
      },
    )
  }

  getEntries() : DataEntry[] {
    return Entries;
  }
}
