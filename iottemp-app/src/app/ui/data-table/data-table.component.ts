import { Component, ViewChild, OnInit } from '@angular/core';
import { DataEntry } from 'src/app/types/DataEntry';
import { DataService } from '../../services/data.service';
import { MatPaginator, PageEvent } from '@angular/material/paginator';

@Component({
  selector: 'app-data-table',
  templateUrl: './data-table.component.html',
  styleUrls: ['./data-table.component.css']
})
export class DataTableComponent implements OnInit {

  entries: DataEntry[] = [];
  activeViewEntries: DataEntry[] = [];

  @ViewChild(MatPaginator) paginator?: MatPaginator;

  length = 0;
  pageSize = 0;
  pageSizeOptions: number[] = [5, 10, 25];

  displayedColumns: string[] = [ "datetime", "sensor_id", "sensor_type", "value" ];

  constructor(private dataService: DataService) {
    this.pageSize = this.pageSizeOptions[this.pageSizeOptions.length - 1];
  }

  onPageChanged(e : PageEvent) : void {
    let firstCut = e.pageIndex * e.pageSize;
    let secondCut = firstCut + e.pageSize;
    this.activeViewEntries = this.entries.slice(firstCut, secondCut);
  }

  ngOnInit() : void {
    this.dataService.getEntries().subscribe(data => {
      this.entries = data;
      this.length = data.length;
      this.activeViewEntries = this.entries.slice(0, this.pageSize);
    });
  }
}
