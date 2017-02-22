/**
* (c)2016 Andreas Seiderer
*/


class PixiChart {
    
    constructor (elementID, opt) {
      this.stage = null;
      this.renderer = null;
      this.graphics = [null,null];
      this.text = null;
      this.front_idx = 1;
      this.back_idx = 0;
      
      this.pos = 0;
      
      this.opt_step = 5;
      this.opt_maxY = 1024;
      this.opt_dataDim = 4;
      this.opt_colors = [0xFF0000, 0x00BB00, 0x00AAEE, 0xEEAA00];
      this.opt_type = 'line';
      this.opt_showText = false;

      if (opt != undefined) {
        if (opt.step != undefined) this.opt_step = opt.step;
        if (opt.maxY != undefined) this.opt_maxY = opt.maxY;
        if (opt.dataDim != undefined) this.opt_dataDim = opt.dataDim;
        if (opt.colors != undefined) this.opt_colors = opt.colors;
        if (opt.type != undefined) this.opt_type= opt.type;
        if (opt.showText != undefined) this.opt_showText = opt.showText;
      }
      
      
      this.lastYval = new Array(this.opt_dataDim);
              
      this.stage = new PIXI.Container();
      this.renderer = PIXI.autoDetectRenderer(window.innerWidth-30, 250, {antialias: true});  //antialiasign
      this.renderer.backgroundColor = 0xEEEEEE;

      document.getElementById(elementID).appendChild(this.renderer.view);
            
      this.graphics[0] = new PIXI.Graphics();
      this.graphics[1] = new PIXI.Graphics();
        
      this.stage.addChild(this.graphics[0]);
      this.stage.addChild(this.graphics[1]);
      
      if (this.opt_showText) {
          
          this.text = new Array(this.opt_dataDim);
          
          for (var i = 0; i < this.opt_dataDim; i++) {
            this.text[i] = new PIXI.Text('',{fontFamily : 'Arial', fontSize: '12px', fill : 0xff1010, align : 'center'});
            this.text[i].position.y = this.renderer.height - (i * (this.renderer.height / this.opt_dataDim) + (this.renderer.height / this.opt_dataDim));
            
            this.stage.addChild(this.text[i]);
          }
      }
            
      this.renderer.render(this.stage);
    }
    
    
    push(val, active) {
        if (this.pos+this.opt_step > this.renderer.width) {
            this.pos = 0;
            this.stage.swapChildren(this.graphics[0],this.graphics[1]);
            
            var temp =  this.front_idx;
            this.front_idx = this.back_idx;
            this.back_idx = temp;
            this.graphics[ this.front_idx].clear();
        }
          
        //background
        this.graphics[ this.back_idx].beginFill(0xFFFFFF);
        this.graphics[ this.back_idx].lineStyle(0, 0xFFFFFF);
        this.graphics[ this.back_idx].drawRect(this.pos, 0, this.opt_step, this.renderer.height);
        this.graphics[ this.back_idx].endFill();
        
        
        //foreground
        for (var i = 0; i < this.opt_dataDim; i++) {
    
            var cur_val = val[i];
                
            if (active[i]) {
                
                if (this.opt_type == 'line') {
                    this.graphics[ this.front_idx].beginFill(0x00FF00);    
                    this.graphics[ this.front_idx].lineStyle(2, this.opt_colors[i]);
                    
                    this.graphics[ this.front_idx].moveTo(this.pos,this.renderer.height-(this.renderer.height/this.opt_maxY*this.lastYval[i]));
                    this.graphics[ this.front_idx].lineTo(this.pos+this.opt_step, this.renderer.height-(this.renderer.height/this.opt_maxY*cur_val));

                    this.graphics[ this.front_idx].endFill();
                    
                } else if (this.opt_type == 'bars') {
                    var maxBarLength = this.renderer.height/this.opt_dataDim;
                    
                    this.graphics[ this.front_idx].beginFill(0x00FF00);    
                    this.graphics[ this.front_idx].lineStyle(2, this.opt_colors[i]);
                    
                    this.graphics[ this.front_idx].moveTo(this.pos+this.opt_step,this.renderer.height-(i*maxBarLength));
                    this.graphics[ this.front_idx].lineTo(this.pos+this.opt_step, this.renderer.height-((i*maxBarLength) + (maxBarLength/this.opt_maxY*cur_val)));
                    
                    this.graphics[ this.front_idx].endFill();
                    
                    if (this.opt_showText) {
                        this.text[i].position.x = this.pos+this.opt_step + 3;
                        this.text[i].text = cur_val;
                    }
                }
            }
            
            this.lastYval[i] = cur_val;

        }


        //position line
        this.graphics[this.back_idx].beginFill(0x000000);
        this.graphics[this.back_idx].lineStyle(2, 0x000000);
        
        this.graphics[this.back_idx].moveTo(this.pos+this.opt_step+1, 0);
        this.graphics[this.back_idx].lineTo(this.pos+this.opt_step+1, this.renderer.height);
        
        this.graphics[this.back_idx].endFill();
        
        
        this.pos+=this.opt_step;

        //render
        this.renderer.render(this.stage);
      }
 };