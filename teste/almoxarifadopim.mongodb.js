// PRIMEITA ETAPA - Criando o banco e a collection.
use('almoxarifado');

// db.produtos.drop();
// Comando de cima é para zerar o banco de dados, para reapresentar. ()

db.createCollection('produtos');


// __________________________________________
// Segunda etapa - Registrar itens (CREATE) - 

// 2.1 Registrar um item
db.produtos.insertOne({
  codigo: 101,
  nome: 'Parafuso M6',
  dataEntrada: new Date('2026-09-10'),
  valorUnitario: 0.50,
  quantidade: 500,
  ativo: true
});

// 2.2 Registrar vários itens de uma vez
db.produtos.insertMany([
  { codigo: 102, nome: 'Arruela', dataEntrada: new Date('2026-09-10'),
    valorUnitario: 0.10, quantidade: 1000, ativo: true },
  { codigo: 103, nome: 'Luva de Seguranca', dataEntrada: new Date('2026-09-12'),
    valorUnitario: 12.90, quantidade: 40, ativo: true },
  { codigo: 104, nome: 'Capacete', dataEntrada: new Date('2026-09-15'),
    valorUnitario: 35.00, quantidade: 8, ativo: true }
]);


// ________________________________
// Terceira etapa - Consultar (READ)

// 3.1 Listar todos os produtos ativos
db.produtos.find({ ativo: true });

// 3.2 Buscar um produto pelo código
db.produtos.findOne({ codigo: 101 });

// 3.3 Produtos com estoque baixo (menos de 10 unidades)
db.produtos.find({ ativo: true, quantidade: { $lt: 10 } });


// ______________________________
// Quarta etapa - Alterar (UPDATE)

// 4.1 Alterar o valor de um produto
db.produtos.updateOne(
  { codigo: 101 },
  { $set: { valorUnitario: 0.55 } }
);

// Conferir a alteração
db.produtos.findOne({ codigo: 101 });


// _______________________________________
// Quinta etapa - Movimentação de estoque

// 5.1 ENTRADA: chegaram 20 capacetes (8 para 28)
db.produtos.updateOne(
  { codigo: 104 },
  { $inc: { quantidade: 20 } }
);

// Conferir
db.produtos.findOne({ codigo: 104 });

// 5.2 SAÍDA: retirar 5 luvas, só se tiver 5 ou mais (40 para 35)
db.produtos.updateOne(
  { codigo: 103, quantidade: { $gte: 5 } },
  { $inc: { quantidade: -5 } }
);

// 5.3 TESTE DE FALHA: tentar retirar 100 capacetes (só tem 28)
// Esperado: matchedCount 0 e modifiedCount 0 (sem alterações)
db.produtos.updateOne(
  { codigo: 104, quantidade: { $gte: 100 } },
  { $inc: { quantidade: -100 } }
);

// Conferir que o estoque continua em 28
db.produtos.findOne({ codigo: 104 });


// ______________________________
// Sexta etapa - Deletar (DELETE)

// 6.1 Exclusão lógica: o item sai das listagens, mas continua no banco (famoso "Salva estagiário kkk")
db.produtos.updateOne(
  { codigo: 102 },
  { $set: { ativo: false } }
);

// Conferir: a Arruela nao deve aparecer
db.produtos.find({ ativo: true });

// 6.2 Exclusão física: apaga o documento de vez (aqui lascou, sobe no RH)
db.produtos.deleteOne({ codigo: 102 });


// ________________________________________________
// Sétima etapa - Relatorio: valor total em estoque

db.produtos.aggregate([
  { $match: { ativo: true } },
  {
    $group: {
      _id: null,
      valorTotal: { $sum: { $multiply: ['$valorUnitario', '$quantidade'] } },
      totalDeItens: { $sum: 1 },
      totalDeUnidades: { $sum: '$quantidade' }
    }
  }
]);


// __________________________________________
// Oitava etapa - Regras de proteção do banco

// 8.1 Índice único: impede dois produtos com o mesmo código
db.produtos.createIndex({ codigo: 1 }, { unique: true });       // OBS: deve dar erro E11000 duplicate key


// 8.2 Validação dos campos (schema)
db.runCommand({
  collMod: 'produtos',
  validator: {
    $jsonSchema: {
      bsonType: 'object',
      required: ['codigo', 'nome', 'dataEntrada', 'valorUnitario', 'quantidade', 'ativo'],
      properties: {
        codigo:        { bsonType: 'number', minimum: 1 },
        nome:          { bsonType: 'string', minLength: 2 },
        dataEntrada:   { bsonType: 'date' },
        valorUnitario: { bsonType: 'number', minimum: 0 },
        quantidade:    { bsonType: 'number', minimum: 0 },
        ativo:         { bsonType: 'bool' }
      }
    }
  },
  validationLevel: 'strict',
  validationAction: 'error'
});

// Caso teste, deve dar erro: quantidade negativa

// O segundo deve aparecer como: produto válido
db.produtos.insertOne({
  codigo: 105,
  nome: 'Fita Isolante',
  dataEntrada: new Date(),
  valorUnitario: 4.50,
  quantidade: 60,
  ativo: true
});

// Conferência final: lista tudo que esta ativo!
db.produtos.find({ ativo: true });
