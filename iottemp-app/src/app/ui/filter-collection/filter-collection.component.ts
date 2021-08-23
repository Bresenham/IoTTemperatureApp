import { Component, ComponentFactory, ComponentFactoryResolver, ComponentRef, OnInit, ViewChild, ViewContainerRef } from '@angular/core';
import { FilterComponent } from '../filter/filter.component';


@Component({
  selector: 'app-filter-collection',
  templateUrl: './filter-collection.component.html',
  styleUrls: ['./filter-collection.component.css']
})
export class FilterCollectionComponent implements OnInit {

  filters: Array<ComponentRef<FilterComponent>> = [];

  @ViewChild("messagecontainer", { read: ViewContainerRef }) entry!: ViewContainerRef;

  constructor(private resolver: ComponentFactoryResolver) { }

  onAddDateRangeFilter() {
    const factory: ComponentFactory<FilterComponent> = this.resolver.resolveComponentFactory(FilterComponent);
    const filter = this.entry.createComponent(factory);
    filter.instance.message = "Date Range Filter";
    this.filters.push(filter);
  }

  ngOnInit(): void {
  }

}
