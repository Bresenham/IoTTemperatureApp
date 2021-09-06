import { Component, OnInit } from '@angular/core';
import { EChartsOption } from 'echarts';
import { TooltipComponentOption } from 'echarts/components'

import { DataEntry } from 'src/app/types/DataEntry';
import { DataService } from '../../services/data.service';

type TooltipFormatterCallback = Exclude<NonNullable<TooltipComponentOption['formatter']>, string>
// single and multiple params
type TooltipFormatterParams = Parameters<TooltipFormatterCallback>[0]

const dataFormatter: TooltipFormatterCallback = (params: TooltipFormatterParams) => {
  if (Array.isArray(params)) {
    return params[0].data.toString() || 'no value';
  }
  return params.data.toString() || '';
}


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
    this.dataService.getEntries().subscribe(data => {
      this.entries = data;

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
        tooltip: {
          formatter: dataFormatter
        },
        series: [
          {
            data: values,
            type: 'line',
          },
        ],
      };
    });
  }

}
