import { Component, OnInit, ViewChild } from '@angular/core';
import { ECharts, EChartsOption, SeriesOption } from 'echarts';
import { TooltipComponentOption } from 'echarts/components'

import { DataEntry } from 'src/app/types/DataEntry';
import { DataService } from '../../services/data.service';

type TooltipFormatterCallback = Exclude<NonNullable<TooltipComponentOption['formatter']>, string>
// single and multiple params
type TooltipFormatterParams = Parameters<TooltipFormatterCallback>[0]

const dataFormatter: TooltipFormatterCallback = (params: TooltipFormatterParams) => {
  if (Array.isArray(params)) {
    return (params[0].data as any[])[1].toString() || 'no value';
  }
  return (params.data as any[])[1].toString() || '';
}

const colors: string[] = ["#21ba45", "#009C95"];

class ChartLineSeries {
  color: string = "#21ba45";
  name: string = "Name";
  type: string = "line";
  smooth: boolean = false;
  symbol: string = "none";
  areaStyle: any = {};
  data: any[] = [[]];
}

@Component({
  selector: 'app-graph',
  templateUrl: './graph.component.html',
  styleUrls: ['./graph.component.css']
})
export class GraphComponent implements OnInit {

  entries: DataEntry[] = [];

  chartOption: EChartsOption = {};

  constructor(private dataService: DataService) { }

  ngOnInit(): void {
    this.chartOption = {
      calculable : true,
      yAxis: {
        type: 'value',
        boundaryGap: [0, '100%']
      },
      xAxis: {
        type: 'time',
        boundaryGap: false
      },
      dataZoom: [{
        type: 'inside',
        start: 0,
        end: 100
      }, {
        start: 0,
        end: 100
      }],
      tooltip: {
        trigger: "axis",
        formatter: dataFormatter
      },
      legend: {
        data: []
      },
      series: []
    };

    this.dataService.dataFilteredObservable.subscribe(data => {
      this.entries = data;

      let seriesMap = new Map<number, ChartLineSeries>();
      this.entries.forEach(entry => {
        if(!seriesMap.has(entry.sensor._id)) {
          let cls : ChartLineSeries = new ChartLineSeries();
          cls.color = colors[entry.sensor._id % colors.length];
          cls.name = `Sensor${entry.sensor._id}`;
          seriesMap.set(entry.sensor._id, cls);
        }
        seriesMap.get(entry.sensor._id)!.data.push([entry.datetime.toString(), entry.value]);
      });

      let series = Array.from(seriesMap.values());
      let legendData = series.map(ser => ser.name);

      this.chartOption.legend = { data: legendData };
      this.chartOption.series = series as SeriesOption[];
      // Trigger echart update
      this.chartOption = Object.assign({}, this.chartOption);
    });
  }

}
