import { Component, OnInit, ViewChild } from '@angular/core';
import { FilterComponent } from '../filter/filter.component';
import { DataService } from '../../services/data.service';
import { DataEntry } from 'src/app/types/DataEntry';

export enum FilterType {
  DateRangeFilter, SensorSelectFilter
}

@Component({
  selector: 'app-filter-collection',
  templateUrl: './filter-collection.component.html',
  styleUrls: ['./filter-collection.component.css']
})
export class FilterCollectionComponent implements OnInit {

  _FilterType = FilterType;
  completeData : DataEntry[] = [];

  @ViewChild('dateRangeFilter') dateRangeFilter? : FilterComponent;

  constructor(private dataService: DataService) { }

  onFilterApply() : void {
    let filter1Data : DataEntry[] = this.dateRangeFilter!.filterData(this.completeData);
    this.dataService.dataFilteredObservable.next(filter1Data);
  }

  ngOnInit(): void {
    this.dataService.getEntries().subscribe(data => {
      this.completeData = data;
      this.dataService.dataFilteredObservable.next(this.completeData);
    });
  }

}
