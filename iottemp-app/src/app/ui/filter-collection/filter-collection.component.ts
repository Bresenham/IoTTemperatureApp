import { Component, ComponentFactory, ComponentFactoryResolver, ComponentRef, OnInit, ViewChild, ViewContainerRef } from '@angular/core';
import { FilterComponent } from '../filter/filter.component';

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

  constructor(private resolver: ComponentFactoryResolver) { }

  ngOnInit(): void {

  }

}
