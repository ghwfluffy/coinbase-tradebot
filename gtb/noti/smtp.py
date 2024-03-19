import json
import smtplib

from gtb.utils.logging import Log

from email.mime.text import MIMEText

def send_email(recipient_email: str, subject: str, message: str) -> bool:
    credential: dict
    with open("secrets/mail.json", "r") as fp:
        credential = json.loads(fp.read())

    try:
        with smtplib.SMTP(credential['server'], credential['port']) as server:
            server.starttls()
            server.login(credential['username'], credential['password'])
            msg: MIMEText = MIMEText(message)
            msg['Subject'] = ""
            msg['From'] = credential['username']
            server.sendmail(credential['username'], recipient_email, msg.as_string())
            server.close()
            return True
    except Exception as e:
        Log.exception("Failed to send email", e)

    return False
