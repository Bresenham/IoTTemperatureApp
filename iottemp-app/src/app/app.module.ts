import { NgModule } from '@angular/core';
import { BrowserModule } from '@angular/platform-browser';

import { NgxEchartsModule } from 'ngx-echarts';

import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { HttpClientModule  } from '@angular/common/http'

import { AppRoutingModule } from './app-routing.module';
import { HeaderComponent } from './ui/header/header.component';
import { AppComponent } from './app.component';
import { GraphComponent } from './ui/graph/graph.component';
import { DataTableComponent } from './ui/data-table/data-table.component';
import { BrowserAnimationsModule } from '@angular/platform-browser/animations';
import { MaterialModule } from './material.module';
import { FilterCollectionComponent } from './ui/filter-collection/filter-collection.component';
import { FilterComponent } from './ui/filter/filter.component';
import { MatPaginatorModule } from '@angular/material/paginator';
import { MatIconModule } from '@angular/material/icon';

@NgModule({
  declarations: [
    AppComponent,
    HeaderComponent,
    GraphComponent,
    DataTableComponent,
    FilterCollectionComponent,
    FilterComponent
  ],
  imports: [
    BrowserModule,
    AppRoutingModule,
    BrowserAnimationsModule,
    MaterialModule,
    ReactiveFormsModule,
    FormsModule,
    MatIconModule,
    MatPaginatorModule,
    HttpClientModule,
    NgxEchartsModule.forRoot({
      /**
       * This will import all modules from echarts.
       * If you only need custom modules,
       * please refer to [Custom Build] section.
       */
      echarts: () => import('echarts'), // or import('./path-to-my-custom-echarts')
    })
  ],
  providers: [],
  bootstrap: [AppComponent]
})
export class AppModule { }
