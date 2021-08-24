import { Component, OnInit } from '@angular/core';
import { DataEntry } from 'src/app/types/DataEntry';
import { DataService } from '../../services/data.service';

@Component({
  selector: 'app-data-table',
  templateUrl: './data-table.component.html',
  styleUrls: ['./data-table.component.css']
})
export class DataTableComponent implements OnInit {

  entries: DataEntry[] = [];

  displayedColumns: string[] = [ "datetime", "sensor_id", "sensor_type", "value" ];

  constructor(private dataService: DataService) { }

  ngOnInit(): void {
    this.entries = this.dataService.getEntries();
  }

}
