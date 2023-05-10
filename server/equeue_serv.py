import uuid
import json
import random
from datetime import datetime, timedelta

from flask import Flask, request, jsonify, abort
app = Flask(__name__)

# переменные для каждой сессии
tokens = []
cashiers = {}

#cashiers['1'] = {'tokens':['1234567890'], 'status': 0}

# Переменные для генерации информации
visitors = []
# Число посетителей на начало сессиии
n_vis = random.randint(0, 10)
# Функция создания нового посетителя
def new_visitor(i):
    visitor = {}
    visitor['id'] = i
    visitor['number'] = i+100
    visitor['type'] = random.randint(0, 1)
    visitor['order_number'] = random.randint(100000, 999999)
    visitor['status'] = 0
    d1 = datetime.now() + timedelta(seconds=10 * i)
    visitor['created_at'] = str(d1.strftime('%Y-%m-%d %H:%M:%S'))
    return visitor

# Заполним информацию о посетителях
for i in range(n_vis):
    visitor = new_visitor(i)
    visitors.append(visitor)

def change_cashier(number, token=None, status=None):    
    if number in cashiers.keys():
        if token is not None:
            cashiers[number]['tokens'].append(token)
        if status is not None:
            cashiers[number]['status'] = status
    else:
        cashiers[number] = {'tokens':[token], 'status': 0}

# Поиск посетителя в списке по id
def get_visitor(vid, status=0):
    visitor = [v for v in visitors if 'id' in v.keys() and v['id'] == int(vid)]
    if len(visitor) == 1:
        if status:
            visitor[0]['status'] = status
        return visitor[0]
    else:
        return 0

# Поиск следующего посетителя
def get_next_visitor():
    print(visitors)
    wait_visitors = [v for v in visitors if v['status'] == 0]
    if len(wait_visitors) > 0:
        print('wait_visitors[0]')
        print(wait_visitors[0])
        return wait_visitors[0]
    else:
        return None

@app.errorhandler(400)
def resource_not_found(e):
    return jsonify(error=str(e)), 400

@app.route('/cashier/<number>', methods = ['POST', 'GET','PATCH'])
def do_cashier(number):
    # Отправляем токен
    if request.method == 'GET':
        if number in cashiers.keys() and 'status' in cashiers[number].keys():
            token = request.headers.get('Authorization')
            if 'tokens' in cashiers[number].keys() and token in cashiers[number]['tokens']:
                return json.dumps({'status': cashiers[number]['status'],
                    'visitor': get_next_visitor(), 
                    'visitor_count': len([v for v in visitors if v['type'] == 0 and v['status'] == 0]), 
                    'visitor_reservation_count': len([v for v in visitors if v['type'] == 1 and v['status'] == 0])})
            else:
                abort(400, description="Ошибка аутентификации")
        else:
            abort(400, description="Касса с таким номером не создавалась или нет поля статус")
    # Отправляем токен
    if request.method == 'POST':
        token = uuid.uuid4().hex
        change_cashier(number, token)
        tokens.append(token)
        print(token)       
        return json.dumps({'token': token})
    # Изменяем статус кассы [Активна / Не активна]
    if request.method == 'PATCH':
        print('DATA ----')
        if number in cashiers.keys() and 'status' in cashiers[number].keys():
            token = request.headers.get('Authorization')
            #print('token ', token)
            if 'tokens' in cashiers[number].keys() and token in cashiers[number]['tokens']:                
                status = request.form.get('status')
                print('status ', status)
                print(request.form)
                change_cashier(number, None, status)
                return json.dumps({})
            else:
                abort(400, description="Ошибка аутентификации")
        else:
            abort(400, description="Касса с таким номером не создавалась или нет поля статус")

# Нажатие кнопки Следующий
@app.route('/cashier/<number>/invite', methods = ['POST'])
def invite_man(number):
    if request.method == 'POST':  
        if number in cashiers.keys() and 'status' in cashiers[number].keys():
            token = request.headers.get('Authorization')
            if 'tokens' in cashiers[number].keys() and token in cashiers[number]['tokens']:
                print(request.values)
                try:
                    vid = int(request.form.get('visitor_id'))
                except:
                    vid = None
                if vid is not None:
                    visitor = get_visitor(vid)
                    if visitor:
                        return json.dumps(visitor)
                    else:
                        abort(400, description="400:4 Такой пользователь не найден")
                else:
                    if len(visitors) > 0:
                        visitor = get_next_visitor()
                        if visitor is not None:
                            return json.dumps(visitor)
                        else:
                            return abort(400, description="400:3 Нет пользователей в очереди")
                    else:
                        abort(400, description="400:3 Нет пользователей в очереди")
                return json.dumps({'status': cashiers[number]['status']})
            else:
                abort(400, description="400:2 Ошибка аутентификации")
        else:
            abort(400, description="400:1 Касса с таким номером не создавалась или нет поля статус")      

# Получаем список посетителей в json
@app.route('/visitors', methods = ['GET'])
def get_visitors():
    if request.method == 'GET':
        token = request.headers.get('Authorization')
        if token in tokens:            
            return json.dumps({'visitors': [v for v in visitors if v['status'] == 0]})
        else:
            abort(400, description="Ошибка аутентификации")

# Меняем статус у посетителя
@app.route('/visitor/<id>', methods = ['PATCH'])
def update_visitor(id):
    print('PATCH USER')
    global visitors
    print([v['id'] for v in visitors if 'id' in v.keys()])
    print(id)
    if request.method == 'PATCH':
        token = request.headers.get('Authorization')
        if token in tokens:
            status = request.form.get('status')
            visitor = get_visitor(int(id), status)
            print(visitors)
            if visitor:
                return json.dumps({})
            else:
                print("Такой пользователь не найден")
                abort(400, description="Такой пользователь не найден")
        else:
            print("Ошибка аутентификации")
            abort(400, description="Ошибка аутентификации")

# Добавим посетителя динамически
index = n_vis + 1
@app.route('/addvisitor', methods = ['POST'])
def add_visitor():
    global index
    if request.method == 'POST':
        visitor = new_visitor(index)
        visitors.append(visitor)
        
        index = index + 1
        return json.dumps({})

if __name__ == '__main__':    
    app.run(debug = True)