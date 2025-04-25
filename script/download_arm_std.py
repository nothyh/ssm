#! /usr/bin/env python3

# write a script to download the arm std library from the internet
# and because it's dynamic, we can not use http request to download it
# so use selenium to download it

from selenium import webdriver
from selenium.webdriver.firefox.options import Options
from selenium.webdriver.common.by import By

home_url = "https://www.st.com/en/embedded-software/stm32-standard-peripheral-libraries.html"


def download_arm_std(mcu_family: str):
    #broswer_options = Options()
    #broswer_options.add_argument('--ignore-certificate-errors')  # Ignore SSL errors
    #driver = webdriver.Firefox(options=broswer_options)

    options = Options()
    options.accept_insecure_certs = True

    driver = webdriver.Firefox(options=options)


    driver.get(home_url)
    # xpath is /html/body/div[1]/div[1]/main/div[2]/div[4]/div[1]/div[2]/div/div/div/div/div/map
    std_peripheral_lib_map = driver.find_element(By.XPATH, "/html/body/div[1]/div[1]/main/div[2]/div[4]/div[1]/div[2]/div/div/div/div/div/map")
    # print the name to debug
    print(std_peripheral_lib_map.get_attribute("name"))

    map_elements = std_peripheral_lib_map.find_elements(By.TAG_NAME, "area")

    #for element in map_elements:
    #    print(element.get_attribute("title"), element.get_attribute("href"))


    # find the child clement of the map with the mcu_family name
    # STM32F0 Standard Peripheral Library
    title_should_be = f"STM32{mcu_family.upper()} Standard Peripheral Library"
    mcu_family_elements = std_peripheral_lib_map.find_elements(By.XPATH, f"//*[@title='{title_should_be}']")
    #for element in mcu_family_elements:
    #    print(element.get_attribute("href"))
    mcu_family_elements = [element for element in mcu_family_elements 
                           if  element.get_attribute("href").find("stm32-standard-peripheral-libraries") != -1]
    if len(mcu_family_elements) == 0:
        print(f"No {mcu_family} Standard Peripheral Libraries found")
    elif len(mcu_family_elements) > 1:
        print(f"Multiple {mcu_family} Standard Peripheral Libraries found")
    else:
        mcu_family_element = mcu_family_elements[0]
        mcu_family_href = mcu_family_element.get_attribute("href")
        driver.close()
        get_latest(mcu_family_href)
    # close the driver
    driver.quit()
    return 

from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.support.ui import WebDriverWait
def get_latest(href:str):
    driver = webdriver.Firefox()
    driver.get(href)

    wait = WebDriverWait(driver, 10)
    # <div class="st-secondary-nav-tabs margin-bottom-25 is-initialized"></div>
    wait.until(EC.presence_of_element_located((By.CLASS_NAME, "st-secondary-nav-tabs")))
    wait.until(EC.presence_of_element_located((By.ID, "initLightDownload")))
    # wait for the download button to be clickable
    download_btn = wait.until(EC.element_to_be_clickable((By.ID, "initLightDownload")))

# 用 JS 点击绕开交互限制
    driver.execute_script("arguments[0].click();", download_btn)
    while True:
        pass
    #download_btn = driver.find_element(By.XPATH, "//*[@id='initLightDownload']")
    # click the download button
    #download_btn.click()
    # wait for the download to finish
    # wait for 10 seconds
    #driver.implicitly_wait(10)


if __name__ == "__main__":
    download_arm_std("f1")
