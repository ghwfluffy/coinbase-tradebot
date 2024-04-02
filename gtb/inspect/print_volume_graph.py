import io
import base64

import matplotlib.pyplot as plt

from gtb.inspect.volume import create_volume_plot

create_volume_plot()

# Save the plot to a BytesIO object
buf = io.BytesIO()
plt.savefig(buf, format='png')  # Save as PNG to the buffer
plt.close()  # Close the plot

# Seek to the beginning of the BytesIO buffer
buf.seek(0)

# Read the buffer content into a byte array
byte_array = buf.getvalue()

print(base64.b64encode(byte_array).decode('utf-8'))
