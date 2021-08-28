import { Injectable } from '@angular/core'
import { Observable } from 'rxjs';
import { HttpClient } from '@angular/common/http'

import { formatDate } from '@angular/common';

import { DataEntry } from '../types/DataEntry';

@Injectable({
  providedIn: 'root'
})
export class DataService {

  constructor(private http: HttpClient) {}

  getEntries() : Observable<Array<DataEntry>> {
    return this.http.get<Array<DataEntry>>('http://localhost:3000/data')
  }
}
