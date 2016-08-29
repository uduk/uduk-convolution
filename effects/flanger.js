PhaseShifter.prototype.flanger = function (x) {

  var time_delay = 0.044;
  var rate = 1;

  var sin_ref = new Float32Array(x.length);
  for (var i = 0; i < x.length; i++) {
    sin_ref[i] = (Math.sin(2 * Math.PI * i * (rate / 44100))); 
  }

  var max_sample = Math.round(time_delay * 44100);

  var y = new Float32Array(x.length);
  y.fill(0);

  for (var i = 0; i < max_sample; i++) {
    y[i] = x[i];
  }

  var amp = 0.7;
  
  for (var i = max_sample + 1; i < x.length; i++) {
    var cur_sin = Math.abs(sin_ref[i]);
    var cur_delay = Math.ceil(cur_sin * max_sample);
    y[i] = (amp * x[i]) + amp * (x[i - cur_delay]);
  }

  return y;
}
