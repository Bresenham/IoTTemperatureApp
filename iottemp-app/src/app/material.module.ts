import {NgModule} from '@angular/core';

import { MatButtonModule } from '@angular/material/button';
import { MatMenuModule } from '@angular/material/menu'
import { MatCardModule } from '@angular/material/card'
import { MatToolbarModule } from '@angular/material/toolbar'
import { MatTabsModule } from '@angular/material/tabs'
import { MatTableModule } from '@angular/material/table'
import { MatDatepickerModule } from '@angular/material/datepicker'
import { MatNativeDateModule } from '@angular/material/core';
import { MatFormFieldModule } from '@angular/material/form-field';
import { MatInputModule } from '@angular/material/input';
import { MatIconModule } from '@angular/material/icon';
import { MatGridListModule } from '@angular/material/grid-list'
import { MatPaginatorModule } from '@angular/material/paginator';
import { MatSelectModule } from '@angular/material/select';

@NgModule({
    imports: [
        MatButtonModule,
        MatMenuModule,
        MatCardModule,
        MatToolbarModule,
        MatTabsModule,
        MatTableModule,
        MatDatepickerModule,
        MatNativeDateModule,
        MatFormFieldModule,
        MatInputModule,
        MatIconModule,
        MatGridListModule,
        MatPaginatorModule,
        MatSelectModule
    ],
    exports: [
        MatButtonModule,
        MatMenuModule,
        MatCardModule,
        MatToolbarModule,
        MatTabsModule,
        MatTableModule,
        MatDatepickerModule,
        MatNativeDateModule,
        MatFormFieldModule,
        MatInputModule,
        MatIconModule,
        MatGridListModule,
        MatPaginatorModule,
        MatSelectModule
    ]
})
export class MaterialModule {}
