ASM = sjasmplus
ASMFLAGS = --fullpath --nologo

all: test.com hush.com wobble.com zap.com flyby.com shoot.com explode.com jump.com coin.com powerup.com twotone.com wail.com yelp.com airraid.com

%.com: %.asm
	$(ASM) $(ASMFLAGS) $< --raw=$@
