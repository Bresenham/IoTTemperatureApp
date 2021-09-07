import { Component, Input, OnInit, Output } from '@angular/core';
import { FilterType } from '../filter-collection/filter-collection.component'
import { FormGroup, FormControl } from '@angular/forms';
import { Sensor } from 'src/app/types/Sensor';
import { DataService } from 'src/app/services/data.service';
import { DataEntry } from 'src/app/types/DataEntry';

@Component({
  selector: 'app-filter',
  templateUrl: './filter.component.html',
  styleUrls: ['./filter.component.css']
})
export class FilterComponent {

  @Input() filter!: FilterType;

  _FilterType = FilterType;

  sensors : Sensor[] = [];
  selectedSensor = "";

  range = new FormGroup({
    start: new FormControl(),
    end: new FormControl()
  });

  constructor(private dataService: DataService) { }

  filterData(data : DataEntry[]) : DataEntry[] {
    // Dummy filter
    return data.slice(0, data.length - 1);
  }

  ngOnInit(): void {
    this.dataService.getSensors().subscribe(sensors => {
      this.sensors = sensors;
    });
  }

}
