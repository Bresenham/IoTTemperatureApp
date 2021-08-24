import { Component, OnInit } from '@angular/core';
import { EChartsOption } from 'echarts';

import { DataEntry } from 'src/app/types/DataEntry';
import { DataService } from '../../services/data.service';

@Component({
  selector: 'app-graph',
  templateUrl: './graph.component.html',
  styleUrls: ['./graph.component.css']
})
export class GraphComponent implements OnInit {

  entries: DataEntry[] = [];

  constructor(private dataService: DataService) { }

  chartOption!: EChartsOption;

  ngOnInit(): void {
    this.entries = this.dataService.getEntries();

    const date_times: string[] = this.entries.map(e => e.datetime);
    const values: number[] = this.entries.map(e => e.value);

    this.chartOption = {
      xAxis: {
        type: 'category',
        data: date_times,
      },
      yAxis: {
        type: 'value',
      },
      series: [
        {
          data: values,
          type: 'line',
        },
      ],
    };
  }

}
