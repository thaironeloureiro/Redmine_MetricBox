<?php header ('Content-type: text/html; charset=UTF-8'); ?>
<?php
error_reporting(E_ERROR);
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
 

//Listar Status
/*
$client->issue_status->all();
$st=$client->issue_status->listing();
echo "<pre>";
print_r($st);
echo "</pre>";
*/
/*listas de status:
    [Novo] => 1
    [Aberto] => 2
    [Andamento] => 4
    [Pendente] => 10
    [Em desenvolvimento] => 12
    [Resolvido] => 3
    [Homologacao] => 28
    [Validado] => 20
    [Avaliar Producao] => 56
    [Rejeitado] => 41
    [Aguardando Informacao] => 22
    [Cancelado] => 6
    
    [Andamento - VAL] => 76
    [Rejeitado - VAL] => 77
    [Aberto - HML] => 78
    [Desenvolvimento: A Fazer] => 88
    [Desenvolvimento: Fazendo] => 89
    [Desenvolvimento: Feito] => 90
    
    [Homologação: Fazendo] => 91
    [Homologação: Feito] => 92
    
    [SD:Prospect:Novo] => 99
    [SD:Prospect:Analisando] => 100
    [SD:Prospect:Pronto] => 111
    [SD:Dev:A fazer] => 102
    [SD:Dev:Fazendo] => 103
    [SD:Dev:Testando] => 105
    [SD:Dev:Pronto] => 112
    [SD:Dev:Homologando] => 106
    [SD:Dev:Disponível] => 113
    [SD:Deploy:Testando Integrado] => 107
    [SD:Deploy:Publicado] => 108
    [SD:OA:Acompanhando] => 109
    [SD:OA:Feito] => 104
    [SD:Cancelado] => 110
    [Backlog: Novo] => 120
    [Backlog: Aberto] => 119
    
    [Desenvolvimento: Em Andamento] => 114
    [Desenvolvimento: Resolvido] => 115
    [Homologação: Em Andamento] => 116
    [Homologação: Validado] => 117
    [Produção: Avaliar Produção] => 118
    [Fechado] => 5*/


$c=$client->issue->all([       
   'project_id' => 'time-traiket',
'query_id' => 9197,  
'limit'=>4000
]);

$exibir_nao_acumulados=0;
$aculumarFases=0;

$vetChamados[]=0;

$data_fim = date('W', strtotime("now"));
$data_fim=$data_fim-1;
$data_ini = $data_fim-10;

$vetkanban['project_id']='time-traiket';
$vetkanban['id_filtro_redmine']='9197';
$vetkanban['data_ini']=$data_ini;
$vetkanban['data_fim']=$data_fim;
$vetkanban['maximo']=0;
$vetkanban['exibir_nao_acumulados']=$exibir_nao_acumulados;
$fatorDt=$data_ini;
for ($Dtx=0; $Dtx<11 ; $Dtx++)
{
   $vetkanban['fechado'][$fatorDt]=0;
   $vetkanban['avaliar_producao'][$fatorDt]=0;
   $vetkanban['homologacao'][$fatorDt]=0;
   $vetkanban['desenvolvimento'][$fatorDt]=0;
   $vetkanban['backlog'][$fatorDt]=0;

   if($exibir_nao_acumulados){
      $vetkanban['avaliar_producao_simples'][$fatorDt]=0;
      $vetkanban['fechado_simples'][$fatorDt]=0;
      $vetkanban['homologacao_simples'][$fatorDt]=0;
      $vetkanban['desenvolvimento_simples'][$fatorDt]=0;
      $vetkanban['backlog_simples'][$fatorDt]=0;
   }
   $fatorDt=$fatorDt+1;      
}

$vetkanban['fechado']['max']=-1;
$vetkanban['fechado']['min']=-1;
$vetkanban['avaliar_producao']['max']=-1;
$vetkanban['avaliar_producao']['min']=-1;
$vetkanban['homologacao']['max']=-1;
$vetkanban['homologacao']['min']=-1;
$vetkanban['desenvolvimento']['max']=-1;
$vetkanban['desenvolvimento']['min']=-1;
$vetkanban['backlog']['max']=-1;
$vetkanban['backlog']['min']=-1;

$vetChamados[]=0;
foreach ($c['issues'] as $key => $value) {
    $idChamado=$value['id'];            
    
    $chamado=$client->issue->show($idChamado , [
    'include' => 'journals',    
    ]);
    

    foreach ($chamado['issue']['journals'] as $keyJ => $valueJ) {
        foreach ($valueJ['details'] as $keyD => $valueD) {                             
            
            //fechado e cancelado
            if($valueD['property']=='attr' && $valueD['name']=='status_id' && ($valueD['new_value']==5 ||$valueD['new_value']==6))
            { 
                $data_ref = date('W', strtotime($valueJ['created_on']));
                
                $vetChamados['livre'][$data_ref][$idChamado]['fase']='fechado';
            }

            //avaliar producao
            if($valueD['property']=='attr' && $valueD['name']=='status_id' && ($valueD['new_value']==118) )
            {                 
                $data_ref = date('W', strtotime($valueJ['created_on']));
                $vetChamados['livre'][$data_ref][$idChamado]['fase']='avaliar_producao';
            }

            //Homologação
            if($valueD['property']=='attr' && $valueD['name']=='status_id' && ($valueD['new_value']==28 || $valueD['new_value']==116|| $valueD['new_value']==117) ) 
            {                 
                $data_ref = date('W', strtotime($valueJ['created_on']));
                $vetChamados['livre'][$data_ref][$idChamado]['fase']='homologacao';
            }

            //desenvolvimento
            if($valueD['property']=='attr' && $valueD['name']=='status_id' && ($valueD['new_value']==3 || $valueD['new_value']==115 || $valueD['new_value']==114) )
            { 
                $data_ref = date('W', strtotime($valueJ['created_on']));
                $vetChamados['livre'][$data_ref][$idChamado]['fase']='desenvolvimento';
            }

            //backlog
            if($valueD['property']=='attr' && $valueD['name']=='status_id' && ($valueD['new_value']==2 || $valueD['new_value']==119|| $valueD['new_value']==1 || $valueD['new_value']==120) )
            {                 
                $data_ref = date('W', strtotime($valueJ['created_on']));
                $vetChamados['livre'][$data_ref][$idChamado]['fase']='backlog';
            }
        }
    } //fim do fireach dos journals
} //fim do foreach dos chamados


//Fechados
foreach ($vetChamados['livre'] as $keyDt => $valueDt) {
    foreach ($valueDt as $keyChamado => $valueFase) {   
        if($valueFase['fase']=='fechado'){
            //muda chave do vetor para elimina-lo de nova busca
            $vetChamados['visto'][$keyDt][$keyChamado]= $vetChamados['livre'][$keyDt][$keyChamado] ; 
        unset($vetChamados['livre'][$keyDt][$keyChamado]);                                                      
        $vetkanban['fechado'][$keyDt]=$vetkanban['fechado'][$keyDt]+1;        
        if($exibir_nao_acumulados)
            $vetkanban['fechado_simples'][$keyDt]=$vetkanban['fechado_simples'][$keyDt]+1;        
        }   
   }
}


//avaliar_producao
foreach ($vetChamados['livre'] as $keyDt => $valueDt) {
    foreach ($valueDt as $keyChamado => $valueFase) {
    if($valueFase['fase']=='avaliar_producao'){
        //muda chave do vetor para elimina-lo de nova busca
        $vetChamados['visto'][$keyDt][$keyChamado]= $vetChamados['livre'][$keyDt][$keyChamado] ; 
        unset($vetChamados['livre'][$keyDt][$keyChamado]);                  
        $vetkanban['avaliar_producao'][$keyDt]=$vetkanban['avaliar_producao'][$keyDt]+1;        
        if($exibir_nao_acumulados)
          $vetkanban['avaliar_producao_simples'][$keyDt]=$vetkanban['avaliar_producao_simples'][$keyDt]+1;
    }   
   }
}

//homologacao
foreach ($vetChamados['livre'] as $keyDt => $valueDt) {
    foreach ($valueDt as $keyChamado => $valueFase) {
    if($valueFase['fase']=='homologacao'){
        //muda chave do vetor para elimina-lo de nova busca
        $vetChamados['visto'][$keyDt][$keyChamado]= $vetChamados['livre'][$keyDt][$keyChamado] ; 
        unset($vetChamados['livre'][$keyDt][$keyChamado]);          
        $vetkanban['homologacao'][$keyDt]=$vetkanban['homologacao'][$keyDt]+1;        
        if($exibir_nao_acumulados)
            $vetkanban['homologacao_simples'][$keyDt]=$vetkanban['homologacao_simples'][$keyDt]+1;
    }   
   }
}

//desenvolvimento
foreach ($vetChamados['livre'] as $keyDt => $valueDt) {
    foreach ($valueDt as $keyChamado => $valueFase) {
    if($valueFase['fase']=='desenvolvimento'){
        //muda chave do vetor para elimina-lo de nova busca
        $vetChamados['visto'][$keyDt][$keyChamado]= $vetChamados['livre'][$keyDt][$keyChamado] ; 
        unset($vetChamados['livre'][$keyDt][$keyChamado]);          
        $vetkanban['desenvolvimento'][$keyDt]=$vetkanban['desenvolvimento'][$keyDt]+1;        
        if($exibir_nao_acumulados)
            $vetkanban['desenvolvimento_simples'][$keyDt]=$vetkanban['desenvolvimento_simples'][$keyDt]+1;
    }   
   }
}

//backlog
foreach ($vetChamados['livre'] as $keyDt => $valueDt) {
    foreach ($valueDt as $keyChamado => $valueFase) {
    if($valueFase['fase']=='backlog'){
        //muda chave do vetor para elimina-lo de nova busca
        $vetChamados['visto'][$keyDt][$keyChamado]= $vetChamados['livre'][$keyDt][$keyChamado] ; 
        unset($vetChamados['livre'][$keyDt][$keyChamado]);          
        $vetkanban['backlog'][$keyDt]=$vetkanban['backlog'][$keyDt]+1;        
        if($exibir_nao_acumulados)
            $vetkanban['backlog_simples'][$keyDt]=$vetkanban['backlog_simples'][$keyDt]+1;
    }   
   }
}

//Gera os dados de forma acumulada
$acumulado_fechado=0;
$acumulado_avaliar_producao=0;
$acumulado_homologacao=0;
$acumulado_desenvolvimento=0;
$acumulado_backlog=0;

$fatorDt= $vetkanban['data_ini'];
for ($Dtx=0; $Dtx<11 ; $Dtx++)
{
   $acumulado_fechado=$acumulado_fechado+$vetkanban['fechado'][$fatorDt];
   $vetkanban['fechado'][$fatorDt]=$acumulado_fechado;
   //linha abaixo para fins de testes, tornando a fase de fechado constante para avaliar as demais
   //$vetkanban['fechado'][substr($fatorDt, 0,4)."-".substr($fatorDt, 4,2)]=1;

  if($aculumarFases)
    $acumulado_avaliar_producao=$acumulado_avaliar_producao+$vetkanban['avaliar_producao'][$fatorDt];
  else  
    $acumulado_avaliar_producao=$vetkanban['avaliar_producao'][$fatorDt];
  $vetkanban['avaliar_producao'][$fatorDt]=$vetkanban['fechado'][$fatorDt]+$acumulado_avaliar_producao;
  
   if($aculumarFases)
        $acumulado_homologacao=$acumulado_homologacao+$vetkanban['homologacao'][$fatorDt];
    else  
      $acumulado_homologacao=$vetkanban['homologacao'][$fatorDt];
   $vetkanban['homologacao'][$fatorDt]=$vetkanban['avaliar_producao'][$fatorDt]+$acumulado_homologacao;

   if($aculumarFases)
        $acumulado_desenvolvimento=$acumulado_desenvolvimento+$vetkanban['desenvolvimento'][$fatorDt];
    else  
      $acumulado_desenvolvimento=$vetkanban['desenvolvimento'][$fatorDt];
   $vetkanban['desenvolvimento'][$fatorDt]=$vetkanban['homologacao'][$fatorDt]+$acumulado_desenvolvimento;

   if($aculumarFases)
        $acumulado_backlog=$acumulado_backlog+$vetkanban['backlog'][$fatorDt];
    else  
      $acumulado_backlog=$vetkanban['backlog'][$fatorDt];
   $vetkanban['backlog'][$fatorDt]=$vetkanban['desenvolvimento'][$fatorDt]+$acumulado_backlog;

   $vetkanban['maximo']=$vetkanban['backlog'][$fatorDt];
    
   
   $vetkanban['fechado']['max']=$vetkanban['fechado'][$fatorDt];
   if($vetkanban['fechado']['min']<0) $vetkanban['fechado']['min']=$vetkanban['fechado'][$fatorDt];
   
   $vetkanban['avaliar_producao']['max']=$vetkanban['avaliar_producao'][$fatorDt];
   if($vetkanban['avaliar_producao']['min']<0) $vetkanban['avaliar_producao']['min']=0;
   
   $vetkanban['homologacao']['max']=$vetkanban['homologacao'][$fatorDt];   
   if($vetkanban['homologacao']['min']<0) $vetkanban['homologacao']['min']=$vetkanban['homologacao'][$fatorDt];
   
   $vetkanban['desenvolvimento']['max']=$vetkanban['desenvolvimento'][$fatorDt];   
   if($vetkanban['desenvolvimento']['min']<0) $vetkanban['desenvolvimento']['min']=$vetkanban['desenvolvimento'][$fatorDt];
   
   $vetkanban['backlog']['max']=$vetkanban['backlog'][$fatorDt];
   if($vetkanban['backlog']['min']<0) $vetkanban['backlog']['min']=$vetkanban['backlog'][$fatorDt];
    

   $fatorDt=$fatorDt+1;   
   //if(substr($fatorDt,-2)=='13') $fatorDt=$fatorDt+88;
}



    $dif_time_script=date_diff($vetkanban['tempo_execucao_script'], new DateTime(),true);
    $vetkanban['tempo_execucao_script']=$dif_time_script->format('%hhs %imin %sseg');

echo json_encode($vetkanban);
?>