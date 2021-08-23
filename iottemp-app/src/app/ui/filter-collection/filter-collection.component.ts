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

  filters : Array<ComponentRef<FilterComponent>> = [];

  @ViewChild('dynamicFilters', { read: ViewContainerRef }) dynamicInsert!: ViewContainerRef;

  constructor(private resolver: ComponentFactoryResolver) { }

  onAddDateRangeFilter() {
    const factory: ComponentFactory<FilterComponent> = this.resolver.resolveComponentFactory(FilterComponent);
    const filter = this.dynamicInsert.createComponent<FilterComponent>(factory);
    filter.instance.filter = FilterType.DateRangeFilter;
    this.filters.push(filter);
  }

  onAddSensorSelectFilter() {
    const factory: ComponentFactory<FilterComponent> = this.resolver.resolveComponentFactory(FilterComponent);
    const filter = this.dynamicInsert.createComponent<FilterComponent>(factory);
    filter.instance.filter = FilterType.SensorSelectFilter;
    this.filters.push(filter);
  }

  ngOnInit(): void {

  }

}
