function joy.load()
    console.log("=================================")
    local str = joy.utils.des_encrypt("1111111", "liwei")
    console.log(str)
    str = joy.utils.des_decrypt("1111111", str)
    console.log(str)
    console.log("====================")
    -- file encryption/decryption examples are commented out
    console.log("crypt demo done, press Q to quit")
end

function joy.hotfix()
    joy.event.quit();
end

function joy.update(dt)

end

function joy.draw()
end

function joy.quit()
    console.log("crypt demo exit")
end
