<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Strict//EN">

<head>
	<title>PHP Code Runner</title>
	
	<link rel="stylesheet" type="text/css" href="style.css" />
	
	<SCRIPT TYPE="text/javascript">
	<!--
	function setbg(color)
	{
	document.getElementById("styled").style.background=color
	}
	//-->
	</SCRIPT>

</head>

<body>

    <div id="demo-top-bar">

  <div id="demo-bar-inside">


    <div id="demo-bar-buttons">
          </div>

  </div>

</div>

<style type="text/css" style="display: none !important;">
	* {
		margin: 0;
		padding: 0;
	}
	body {
		overflow-x: hidden;
	}
	#demo-top-bar {
		text-align: left;
		background: #222;
		position: relative;
		zoom: 1;
		width: 100% !important;
		z-index: 6000;
		padding: 20px 0 20px;
	}
	#demo-bar-inside {
		width: 960px;
		margin: 0 auto;
		position: relative;
		overflow: hidden;
	}
	#demo-bar-buttons {
		padding-top: 10px;
		float: right;
	}
	#demo-bar-buttons a {
		font-size: 12px;
		margin-left: 20px;
		color: white;
		margin: 2px 0;
		text-decoration: none;
		font: 14px "Lucida Grande", Sans-Serif !important;
	}
	#demo-bar-buttons a:hover,
	#demo-bar-buttons a:focus {
		text-decoration: underline;
	}
	#demo-bar-badge {
		display: inline-block;
		width: 302px;
		padding: 0 !important;
		margin: 0 !important;
		background-color: transparent !important;
	}
	#demo-bar-badge a {
		display: block;
		width: 100%;
		height: 38px;
		border-radius: 0;
		bottom: auto;
		margin: 0;
		background-size: 100%;
		overflow: hidden;
		text-indent: -9999px;
	}
	#demo-bar-badge:before, #demo-bar-badge:after {
		display: none !important;
	}

</style>

	<div id="page-wrap">

		<h1>PHP Code Runner</h1>
		<br />
		
		<form method="post" action="">
			<textarea name="code" id="styled" onfocus="this.value=''; setbg('#e5fff3');" onblur="setbg('white')">Enter your code here</textarea>
		
			<br />
		
			<input type="submit" value="Run!" />
		
			<br />
			<br />
		</form>

		<?php
			if (isset($_POST['code'])) {
				print "<b>Result:</b><br />";
				eval($_POST['code']);
			}
		?>		
	
	</div>


</body>

</html>
