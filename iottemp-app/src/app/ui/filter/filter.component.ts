import { Component, Input, OnInit, Output } from '@angular/core';
import { FilterType } from '../filter-collection/filter-collection.component'
import { FormGroup, FormControl } from '@angular/forms';

@Component({
  selector: 'app-filter',
  templateUrl: './filter.component.html',
  styleUrls: ['./filter.component.css']
})
export class FilterComponent {

  @Input() filter!: FilterType;

  _FilterType = FilterType;

  range = new FormGroup({
    start: new FormControl(),
    end: new FormControl()
  });

  constructor() {
  }

  ngOnInit(): void {

  }

}
