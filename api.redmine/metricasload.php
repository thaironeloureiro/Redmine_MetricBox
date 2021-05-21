<?php header ('Content-type: text/html; charset=UTF-8'); ?>
<?php
require_once 'lib/autoload.php';

$vetkanban['tempo_execucao_script']=new DateTime();
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
 

$filtro=$_GET['filtro'];
$times=split("|", $_GET['time']);

$vetkanban_S = array();

echo  var_dump($times);

foreach ($times as &$time) {

	$c=$client->issue->all([       
	   'project_id' => $time,
	'query_id' => $filtro,  
	'limit'=>4000
	]);

	$vetkanban['Nome_do_Time:']=$time;  
	$vetkanban['query_id']=$filtro;
	$vetkanban['leadtime']=0;
	$vetkanban['novo_a_resolvido']=0;
	$vetkanban['novo_a_aberto']=0;
	$vetkanban['homologacao_a_validado']=0;
	$vetkanban['cycletime_resolvido_fechado']=0;
	$vetkanban['amostra2']=0;


	$qtd_chamados_leadtime=0;
	$qtd_chamados_novo_resolvido=0;
	$qtd_chamados_novo_aberto=0;
	$qtd_chamados_novo_fechado=0;
	$qtd_chamados_homologa_validado=0;
	$qtd_chamados_rejeitados=0;
	$qtd_chamados_resolvido_fechado=0;

	foreach ($c['issues'] as $key => $value) {
		$qtd_chamados_leadtime++;
		$idChamado=$value['id'];            
		$issueId=1; //usar quando não se quer manter os dados por chamados. com isso ira sobrepor os dados no vetor e mantem-se apenas os acumulados

		$vetChamados[]=0;
		$vetChamado[$issueId]['fechado']=""; 
		$vetChamado[$issueId]['resolvido']=""; 
		$vetChamado[$issueId]['aberto']="";
		$vetChamado[$issueId]['validado']="";
		$vetChamado[$issueId]['created_on']=$value['created_on']; 
		$vetChamado[$issueId]['updated_on']=$value['updated_on'];     
		//$vetChamado[$issueId]['closed_on']=$value['closed_on']; 
		
		
		$chamado=$client->issue->show($idChamado , [
		'include' => 'journals',    
		]);

		//Caso precise obter dados da mudança de status
		$tag="";
		foreach ($chamado['issue']['journals'] as $keyJ => $valueJ) {
			foreach ($valueJ['details'] as $keyD => $valueD) {                             
				
				//Resolvido 3 e 90
				if($valueD['property']=='attr' && $valueD['name']=='status_id' && ($valueD['new_value']==3 || $valueD['new_value']==90) )
				{ 
					$vetChamado[$issueId]['resolvido']=$valueJ['created_on'];
				}

				//aberto 2 119
				if($valueD['property']=='attr' && $valueD['name']=='status_id' && ($valueD['new_value']==2 || $valueD['new_value']==119) )
				{ 
					$vetChamado[$issueId]['aberto']=$valueJ['created_on'];
				}

				
				//Homologação 28 e 91
				if($valueD['property']=='attr' && $valueD['name']=='status_id' && ($valueD['new_value']==28 || $valueD['new_value']==91) ) 
				{ 
					$vetChamado[$issueId]['homologacao']=$valueJ['created_on'];
				}

				//Validado 20 e 92
				if($valueD['property']=='attr' && $valueD['name']=='status_id' && ($valueD['new_value']==20 || $valueD['new_value']==92) )
				{ 
					$vetChamado[$issueId]['validado']=$valueJ['created_on'];
				}

				//Fechado 5 
				if($valueD['property']=='attr' && $valueD['name']=='status_id' && $valueD['new_value']==5)
				{ 
					$vetChamado[$issueId]['fechado']=$valueJ['created_on'];
				}

				//rejeicao    
				if($valueD['property']=='attr' && ($valueD['name']=='status_id' && $valueD['new_value']==41) || ($valueD['name']=='tag_list' && strpos($valueD['new_value'], 'Rejeitado')!==false) )
				{ 
					$qtd_chamados_rejeitados++;
				}

			}
		} //fim do fireach dos journals

		//cycletime desenvolvimento [novo - resolvido] em dias, horas, min e ss
		if(strtotime($vetChamado[$issueId]['resolvido'])>0){
			$datetime1=new DateTime(gmdate('Y-m-d h:i:sa',strtotime($value['created_on'])));
			$datetime2=new DateTime(gmdate('Y-m-d h:i:sa',strtotime($vetChamado[$issueId]['resolvido'])));
			$dif_datetime = date_diff($datetime1, $datetime2,true);
			$vetChamado[$issueId]['novo_a_resolvido']=$dif_datetime->format('%a dias %hhs %imin %sseg');
			$vetkanban['novo_a_resolvido']+=(strtotime($vetChamado[$issueId]['resolvido']) - strtotime($value['created_on']));
			$qtd_chamados_novo_resolvido++;                
		}
		if(strtotime($vetChamado[$issueId]['aberto'])>0){
			$datetime1=new DateTime(gmdate('Y-m-d h:i:sa',strtotime($value['created_on'])));
			$datetime2=new DateTime(gmdate('Y-m-d h:i:sa',strtotime($vetChamado[$issueId]['aberto'])));
			$dif_datetime = date_diff($datetime1, $datetime2,true);
			$vetChamado[$issueId]['novo_a_aberto']=$dif_datetime->format('%a dias %hhs %imin %sseg');
			$vetkanban['novo_a_aberto']+=(strtotime($vetChamado[$issueId]['aberto']) - strtotime($value['created_on']));
			$qtd_chamados_novo_aberto++;                
		}


		//cycletime homologação [homologação - validado] em dias, horas, min e ss
		if(strtotime($vetChamado[$issueId]['validado'])>0 && strtotime($vetChamado[$issueId]['homologacao'])>0){
			$datetimeHML1=new DateTime(gmdate('Y-m-d h:i:sa',strtotime($vetChamado[$issueId]['homologacao'])));
			$datetimeHML2=new DateTime(gmdate('Y-m-d h:i:sa',strtotime($vetChamado[$issueId]['validado'])));
			$dif_datetime = date_diff($datetimeHML1, $datetimeHML2,true);
			$vetChamado[$issueId]['homologacao_a_validado']=$dif_datetime->format('%a dias %hhs %imin %sseg');
			$vetkanban['homologacao_a_validado']+=(strtotime($vetChamado[$issueId]['validado']) - strtotime($vetChamado[$issueId]['homologacao']));
			$qtd_chamados_homologa_validado++;
			//echo "<br> teste: "; print_r($datetimeHML1); print_r($datetimeHML2);
		}

		//leadtime  [novo - fechado] 
		if(strtotime($vetChamado[$issueId]['fechado'])>0){
			$datetime1=new DateTime(gmdate('Y-m-d h:i:sa',strtotime($value['created_on'])));
			$datetime2=new DateTime(gmdate('Y-m-d h:i:sa',strtotime($vetChamado[$issueId]['fechado'])));
			$dif_datetime = date_diff($datetime1, $datetime2,true);
			$vetChamado[$issueId]['novo_a_resolvido']=$dif_datetime->format('%a dias %hhs %imin %sseg');
			$vetkanban['leadtime']+=(strtotime($vetChamado[$issueId]['fechado']) - strtotime($value['created_on']));
			$qtd_chamados_novo_fechado++;        
		 
		}


	} //fim do foreach dos chamados
		if($qtd_chamados_novo_resolvido >0){
			$vetkanban['novo_a_resolvido']=abs($vetkanban['novo_a_resolvido']/$qtd_chamados_novo_resolvido);
			$datetime1=new DateTime(gmdate('Y-m-d h:i:sa',strtotime('1970-01-01T00:00:00Z')));
			$datetime2=new DateTime(gmdate('Y-m-d h:i:sa',$vetkanban['novo_a_resolvido']));
			$dif_datetime = date_diff($datetime1, $datetime2,true);    
			$vetkanban['novo_a_resolvido']=$dif_datetime->format("%a dias ");
		}

		if($qtd_chamados_novo_fechado){
			$vetkanban['leadtime']=abs($vetkanban['leadtime']/$qtd_chamados_novo_fechado);
			$datetime1=new DateTime(gmdate('Y-m-d h:i:sa',strtotime('1970-01-01T00:00:00Z')));
			$datetime2=new DateTime(gmdate('Y-m-d h:i:sa',$vetkanban['leadtime']));
			$dif_datetime = date_diff($datetime1, $datetime2,true);    
			$vetkanban['leadtime']=$dif_datetime->format('%a dias %hhs %imin %sseg');
		}

		if($qtd_chamados_novo_aberto >0){
			$vetkanban['novo_a_aberto']=abs($vetkanban['novo_a_aberto']/$qtd_chamados_novo_aberto);
			$datetime1=new DateTime(gmdate('Y-m-d h:i:sa',strtotime('1970-01-01T00:00:00Z')));
			$datetime2=new DateTime(gmdate('Y-m-d h:i:sa',$vetkanban['novo_a_aberto']));
			$dif_datetime = date_diff($datetime1, $datetime2,true);    
			$vetkanban['novo_a_aberto']=$dif_datetime->format('%a dias %hhs %imin %sseg');
		}

		if($qtd_chamados_homologa_validado >0){
			$vetkanban['homologacao_a_validado']=abs($vetkanban['homologacao_a_validado']/$qtd_chamados_homologa_validado);
			$datetime1=new DateTime(gmdate('Y-m-d h:i:sa',strtotime('1970-01-01T00:00:00Z')));
			$datetime2=new DateTime(gmdate('Y-m-d h:i:sa',$vetkanban['homologacao_a_validado']));
			$dif_datetime = date_diff($datetime1, $datetime2,true);    
			$vetkanban['homologacao_a_validado']=$dif_datetime->format('%a dias %hhs %imin %sseg');
		}
		
		$vetkanban['qtd_chamados_leadtime']=$qtd_chamados_novo_fechado;
		$vetkanban['qtd_chamados_novo_resolvido']=$qtd_chamados_novo_resolvido;
		$vetkanban['qtd_chamados_homologa_validado']=$qtd_chamados_homologa_validado;
		$vetkanban['qtd_chamados_resolvido_fechado']=$qtd_chamados_resolvido_fechado;
		$vetkanban['qtd_chamados_novo_aberto']=$qtd_chamados_novo_aberto;

		$dif_time_script=date_diff($vetkanban['tempo_execucao_script'], new DateTime(),true);
		$vetkanban['tempo_execucao_script']=$dif_time_script->format('%a dias %hhs %imin %sseg');
		$vetkanban['qtd_chamados_rejeitados']=$qtd_chamados_rejeitados;
		$vetkanban['taxa_rejeicao']=($qtd_chamados_rejeitados/$qtd_chamados_leadtime)*100;
		$vetkanban['taxa_rejeicao']=round($vetkanban['taxa_rejeicao'],2)."%";
		
		array_push($vetkanban_s, $vetkanban)
	
}
echo json_encode($vetkanban_s);
?>