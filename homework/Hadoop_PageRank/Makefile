JC          := javac
JFLAGS      := 

SRC_DIR     := src
CLASS_DIR   := bin
SRC_FILE    :=  $(SRC_DIR)/*.java

CLASS_PATH  := `hadoop classpath`

TARGET_JAR  := pageRank.jar

all: $(CLASS_DIR) $(TARGET_JAR)

$(CLASS_DIR):
	mkdir -p $@	

$(TARGET_JAR): $(SRC_FILE)
	$(JC) -classpath $(CLASS_PATH) $^ -d $(CLASS_DIR)
	jar -cvf $@ -C $(CLASS_DIR) .

clean:
	rm -rf $(CLASS_DIR) $(TARGET_JAR)
