import {NgModule} from '@angular/core';

import { MatButtonModule } from '@angular/material/button';
import { MatMenuModule } from '@angular/material/menu'
import { MatCardModule } from '@angular/material/card'
import { MatToolbarModule } from '@angular/material/toolbar'
  
@NgModule({
    imports: [
        MatButtonModule,
        MatMenuModule,
        MatCardModule,
        MatToolbarModule
    ],
    exports: [
        MatButtonModule,
        MatMenuModule,
        MatCardModule,
        MatToolbarModule
    ]
})
export class MaterialModule {}
