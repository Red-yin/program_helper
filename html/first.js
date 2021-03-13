var proc_info = new Vue({
	el: '#proc_info',
	data: {
		message: "input process name",
		params:[],
		timerID: 0
	},
	methods: {
		get_process_info: function(param){
			var xmlhttp;
			console.log(param)
			if(window.XMLHttpRequest)
			{
				//code for IE7+,Firefox,Chrome,Opera,Safari
				xmlhttp=new XMLHttpRequest();
			}
			else
			{
				//code for IE6,IE5
				xmlhttp=new ActiveXObject("Microsoft.XMLHTTP");
			}
			xmlhttp.onreadystatechange=function()
			{
				if(xmlhttp.readyState==4 && xmlhttp.status==200)
				{
					//将接收到的字符串存入jsonstr
					var jsonstr = xmlhttp.responseText;
					try{
						var j = JSON.parse(jsonstr);
						for(var i = 0; i < j.length; i++){
							console.log(j[i]["name"] + ":")
							this.data.params.push(j[i]);
							for(var k=0; k < j[i]["status"].length; k++){
								console.log(j[i]["status"][k]["key"] + ":" + j[i]["status"][k]["value"])
							}
						}
					}catch(err){
						console.log(jsonstr + "is not json");
					}
				}

			}
			var url = "cgi-bin/memScan.cgi?" + param;
			console.log(url);
			xmlhttp.open("GET", url,true);
			xmlhttp.send();
		},
		start_timer: function(){
			var name_list = document.getElementsByName("proc_name")
			var param = '';
			var index = 0
			for(i = 0; i < name_list.length; i++){
				if(name_list[i].checked){
					if(index > 0){
						param += "&";
					}
					param += name_list[i].value;
					index += 1
				}
			}
			timerID = self.setInterval(this.$options.methods.get_process_info, 1000, param);
		}
	}
});
