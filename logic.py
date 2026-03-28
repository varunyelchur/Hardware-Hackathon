def classify_status(temp, light, sound, tilt, flame):
    if flame == 1 and temp > 50:
        return "CRITICAL", "Fire hazard detected. High temperature and flame signals were both triggered."

    if tilt == 1 and sound > 70:
        return "WARNING", "Impact or disturbance event detected from tilt and abnormal sound."

    if temp > 45:
        return "WARNING", "Temperature is elevated beyond the safe operating range."

    if light < 20 and sound < 10:
        return "WARNING", "Low-light low-activity condition detected. Check environment status."

    return "SAFE", "Environment appears stable. No major hazard detected."