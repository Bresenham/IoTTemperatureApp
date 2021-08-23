import { Component, OnInit } from '@angular/core';

import { formatDate } from '@angular/common';

export interface DataEntry {
  id: number;
  datetime: string;
  sensor_id: number;
  value: number;
}

const TABLE_DATA: DataEntry[] = [

  { id: 1, datetime: formatDate(Date.now(), 'dd/MM/yyyy HH:mm:ss', 'en-US'), sensor_id: 5820205, value: 2502 },
  { id: 2, datetime: formatDate(Date.now(), 'dd/MM/yyyy HH:mm:ss', 'en-US'), sensor_id: 5793410, value: 2242 },
  { id: 3, datetime: formatDate(Date.now(), 'dd/MM/yyyy HH:mm:ss', 'en-US'), sensor_id: 5820205, value: 2729 },

];

@Component({
  selector: 'app-data-table',
  templateUrl: './data-table.component.html',
  styleUrls: ['./data-table.component.css']
})
export class DataTableComponent implements OnInit {

  displayedColumns: string[] = [ "datetime", "sensor_id", "value" ];

  dataSource = TABLE_DATA;

  constructor() { }

  ngOnInit(): void {
  }

}
