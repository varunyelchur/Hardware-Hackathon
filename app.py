import streamlit as st
from logic import classify_status

st.set_page_config(page_title="Autonomous Safety Node", layout="wide")

st.title("Autonomous Safety Node")
st.subheader("Real-time environmental hazard monitoring")

st.sidebar.header("Test Sensor Inputs")

temp = st.sidebar.slider("Temperature (C)", 0, 100, 25)
light = st.sidebar.slider("Light Level", 0, 100, 50)
sound = st.sidebar.slider("Sound Level", 0, 100, 20)
tilt = st.sidebar.selectbox("Tilt Detected", [0, 1])
flame = st.sidebar.selectbox("Flame Detected", [0, 1])

status, explanation = classify_status(temp, light, sound, tilt, flame)

col1, col2 = st.columns(2)

with col1:
    st.metric("Temperature", f"{temp} C")
    st.metric("Light", light)
    st.metric("Sound", sound)

with col2:
    st.metric("Tilt", tilt)
    st.metric("Flame", flame)

st.markdown("## System Status")
if status == "SAFE":
    st.success(status)
elif status == "WARNING":
    st.warning(status)
else:
    st.error(status)

st.markdown("## AI Interpretation")
st.write(explanation)