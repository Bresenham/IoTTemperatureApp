import { Injectable } from '@angular/core'
import { Observable, BehaviorSubject } from 'rxjs';
import { HttpClient } from '@angular/common/http'

import { DataEntry } from '../types/DataEntry';
import { Sensor } from '../types/Sensor';

@Injectable({
  providedIn: 'root'
})
export class DataService {

  constructor(private http: HttpClient) {}

  public dataFilteredObservable: BehaviorSubject<Array<DataEntry>> = new BehaviorSubject<Array<DataEntry>>([]);

  getEntries() : Observable<Array<DataEntry>> {
    return this.http.get<Array<DataEntry>>('http://192.168.0.103:3000/data');
  }

  getSensors() : Observable<Array<Sensor>> {
    return this.http.get<Array<Sensor>>('http://192.168.0.103:3000/sensor');
  }
}
