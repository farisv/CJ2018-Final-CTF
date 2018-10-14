 <?php
  if (!empty($_GET['number'])) {
    $number = $_GET['number'];
    $is_numeric = (is_numeric($number) ? "a number" : "not a number");
    print eval("print '$number is $is_numeric';");
  } else {
    highlight_file(__FILE__);
  }
