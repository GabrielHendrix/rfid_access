from flask import Blueprint, render_template, redirect, url_for, request, flash
from werkzeug.security import generate_password_hash, check_password_hash
from flask_login import login_user, logout_user, login_required
from .models import User
from . import db
import pyodbc

auth = Blueprint('auth', __name__)
def connection():
    # sqlcmd -S localhost,1415 -U SA -P 7u8i9o0P
    s = 'localhost,1415' #Your server name 
    d = 'TblUsers' 
    u = 'SA' #Your login
    p = '7u8i9o0P' #Your login password
    cstr = 'DRIVER={ODBC Driver 17 for SQL Server};SERVER='+s+';DATABASE='+d+';UID='+u+';PWD='+ p
    conn = pyodbc.connect(cstr)
    return conn

@auth.route('/login')
def login():
    return render_template('login.html')

@auth.route('/signup')
def signup():
    return render_template('signup.html')

@auth.route('/logout')
def logout():
    logout_user()
    return redirect(url_for('main.index'))

@auth.route('/signup', methods=['POST'])
def signup_post():
    conn = connection()
    cursor = conn.cursor()
    email = request.form.get('email')
    name = request.form.get('name')
    password = request.form.get('password')
    rfid_id = request.form.get('id')
    user = User.query.filter_by(rfid_id=rfid_id).first() # if this returns a user, then the email already exists in database

    if user: # if a user is found, we want to redirect back to signup page so user can try again
        flash('RFID ID already exists in DB')
        return redirect(url_for('auth.signup'))
    try:
        cursor.execute("INSERT INTO dbo.TblUsers (id, name) VALUES (?, ?)", str(rfid_id), str(name))
        conn.commit()
        conn.close()
    except Exception as e:
        print(e)
    # create a new user with the form data. Hash the password so the plaintext version isn't saved.
    new_user = User(email=email, name=name, rfid_id=rfid_id, password=generate_password_hash(password, method='sha256'))

    # add the new user to the database
    db.session.add(new_user)
    db.session.commit()

    # code to validate and add user to database goes here
    return redirect(url_for('auth.login'))

@auth.route('/login', methods=['POST'])
def login_post():
    email = request.form.get('email')
    password = request.form.get('password')
    remember = True if request.form.get('remember') else False

    user = User.query.filter_by(email=email).first()

    # check if the user actually exists
    # take the user-supplied password, hash it, and compare it to the hashed password in the database
    if not user or not check_password_hash(user.password, password):
        flash('Please check your login details and try again.')
        return redirect(url_for('auth.login')) # if the user doesn't exist or password is wrong, reload the page

    # if the above check passes, then we know the user has the right credentialsi
    login_user(user, remember=remember)
    return redirect(url_for('main.profile'))


