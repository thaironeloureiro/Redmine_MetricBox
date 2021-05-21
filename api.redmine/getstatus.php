<?php header ('Content-type: text/html; charset=UTF-8'); ?>
<?php
ini_set( 'display_errors', 0 );
//echo "header('Content-Type: text/html; charset=ISO-8859-1',true)";


require_once 'lib/autoload.php';



// ----------------------------
// Instantiate a redmine client
// --> with ApiKey
$client = new Redmine\Client('http://localhost', '1234567890abcdfgh');

// ----------------------------
// [OPTIONAL] if you want to check
// the servers' SSL certificate on Curl call
$client->setCheckSslCertificate(true);

// ----------------------------
// [OPTIONAL] set the port
// (it will try to guess it from the url)
$client->setPort(80);


//$issueId=97482;
$issueId=$_GET['issueId'];
$c=$client->issue->show($issueId , array ('include' => 'journals'));


$tag="";

foreach ($c['issue']['journals'] as $key => $value) {		
  foreach ($value['details'] as $key1 => $value1) {  	   	  		
  		if($value1['property']=='attr' && $value1['name']=='tag_list'){
            $tag=$value1['new_value'];
  		}  			    	
  }  		

}


$linha2=$c['issue']['status']['name'];

//Uma simples saída JSON
$object = new stdclass();
$object->chamado = "#".$c['issue']['id'];
$object->projeto = $c['issue']['project']['name'];
$object->categoria = $c['issue']['tracker']['name'];
$object->titulo = $c['issue']['subject'];
$object->situcao = $linha2;
$object->tags = $tag;
$object->prioridade = is_null($c['issue']['priority']['name'])?"---":$c['issue']['priority']['name'];
$object->versao = is_null($c['issue']['fixed_version']['name'])?"---":$c['issue']['fixed_version']['name'];

echo json_encode($object);
?>