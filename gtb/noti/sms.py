import json

from gtb.noti.smtp import send_email

def send_message(message: str) -> bool:
    phone_info: dict
    with open("secrets/sms.json", "r") as fp:
        phone_info = json.loads(fp.read())

    sms_email: str = phone_info['mobile'] + "@" + phone_info['sms_domain']
    return send_email(sms_email, "", message)
