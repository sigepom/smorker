<script language="JavaScript" type="text/javascript">
	function CountdownTimer(tl){
		this.initialize.apply(this,arguments);
	}
	CountdownTimer.prototype={
		initialize:function(elm,tl) {
			this.elem = document.getElementById(elm);
			this.tl = new Date(tl * 1000);
		},countDown:function(){
			var timer = '';
			var today = new Date();

			if(( this.tl - today ) > 0 ){
				var min = Math.floor((this.tl-today+500)/(60*1000));
				var sec = Math.floor((this.tl-today+500)%(60*1000)/1000);
				var me=this;

				timer = '<span class="min">調理時間 : '+this.addZero(min)+'分</span><span class="sec">'+this.addZero(sec)+'秒</span>';
				this.elem.innerHTML = timer;
				tid = setTimeout( function(){me.countDown();},100 );
			}
		},addZero:function(num){ return ('0'+num).slice(-2); }
	}
	function CDT(){
	}
	window.onload=function(){
		var timer = new CountdownTimer('CDT',%time_limit%);
		timer.countDown();
	}
</script>

