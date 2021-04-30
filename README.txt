Seguem alguns tópicos sobre os arquivos nessa pasta.

1) Executar o script para gerar o gráfico:
	$ python3 plot_data.py
	
	Tenha certeza de que todas as bibliotecas estejam instaladas. Caso contrário, o próprio terminal irá solicitar a instalação do que for necessário.
	Caso a execução esteja "travada" ou algum empecilho maior com a geração do gráfico surja, é provável que a taxa de atualização esteja muito elevada (isso depende do computador em que está sendo executado). Dentro do arquivo plot_data.py, altere a linha
	
	# ani = FuncAnimation(fig, animate, interval = 1)
	
	Em interval, aumente o valor para, por exemplo, 10 ou 50. Qunato maior, mais lenta a movimentação do gráfico. Um é o valor mínimo.
	
2) Sobre as pastas internas:
	a) Na principal, têm-se os códigos implementados. Foi deixado dessa maneira porque plot_data.py precisa estar nessa posição.
	b) apresentacao: contém o PDF da apresentação e um link para o vídeo da apresentação no Youtube, bem como uma imagem utilizada durante a gravação do vídeo que demonstra a saída ideal para o projeto.
	c) data: contém um arquivo com os dados usados para gerar o gráfico e o outro foi utilizado para a análise estatística dos dados.
	d) __pycache__: gerada durante a execução do projeto, deve ser deixada como está.
