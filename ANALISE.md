# Análise de Desempenho — HashMap

## Especificações da Máquina

- **CPU:** AMD Ryzen 5 7600 6-Core (12 threads)
- **RAM:** ~30,4 GiB.
- **SO:** Linux 7.3.4

## Resultados (`./build/c_app`)

- **`capacity=1031` (primo):** inserir `1.154.730 ns` (~1,15 ms) |
  buscar `59.192.009 ns` (~59,19 ms)
- **`capacity=1024` (potência de 2):** inserir `1.184.939 ns` (~1,18 ms) |
  buscar `59.741.201 ns` (~59,74 ms)

## Análise

A tabela com capacidade prima (`1031`) obteve desempenho ligeiramente
superior tanto na inserção (~2,5% mais rápida) quanto na busca (~0,9%
mais rápida) em relação à capacidade `1024`, indicando menor taxa de
colisões, embora a diferença prática nesta execução seja pequena.
