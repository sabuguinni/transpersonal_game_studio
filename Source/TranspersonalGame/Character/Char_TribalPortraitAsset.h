#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/Texture2D.h"
#include "Sound/SoundCue.h"
#include "Char_TribalPortraitAsset.generated.h"

// TODO_ASSET_GENERATION_FAILED: DALL-E failed - manual portrait creation required
// Portrait Specifications:
// - Weathered human face showing survival experience
// - Bone jewelry (carved teeth, small animal bones)
// - Animal hide clothing visible at shoulders/neck
// - Earth tone color palette (browns, tans, ochre, weathered skin)
// - Realistic facial features with scars and weathered texture
// - Wise, determined expression
// - Suitable for UI character portraits and dialogue system

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalPortraitData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portrait")
    TSoftObjectPtr<UTexture2D> PortraitTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portrait")
    TSoftObjectPtr<UTexture2D> PortraitIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> GreetingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> CombatSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Info")
    FString CharacterName = TEXT("Kael");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Info")
    FString TribeName = TEXT("Stone Ridge");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Info")
    FText CharacterDescription = FText::FromString(TEXT("A weathered warrior who has survived countless encounters with the great beasts."));

    FChar_TribalPortraitData()
    {
        // Placeholder texture paths - to be replaced when assets are created
        PortraitTexture = TSoftObjectPtr<UTexture2D>(FSoftObjectPath("/Engine/EngineResources/DefaultTexture"));
        PortraitIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath("/Engine/EngineResources/DefaultTexture"));
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UChar_TribalPortraitAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UChar_TribalPortraitAsset();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portrait Configuration")
    FChar_TribalPortraitData PortraitData;

    // Generated TTS audio URL (from ElevenLabs)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generated Audio")
    FString GreetingAudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781377905058_Kael_TribalWarrior.mp3");

    // Get the portrait texture for UI display
    UFUNCTION(BlueprintCallable, Category = "Portrait")
    UTexture2D* GetPortraitTexture();

    // Get the character name for dialogue system
    UFUNCTION(BlueprintCallable, Category = "Character Info")
    FString GetCharacterName() const { return PortraitData.CharacterName; }

    // Get the tribe name
    UFUNCTION(BlueprintCallable, Category = "Character Info")
    FString GetTribeName() const { return PortraitData.TribeName; }

    // Get character description for tooltips/UI
    UFUNCTION(BlueprintCallable, Category = "Character Info")
    FText GetCharacterDescription() const { return PortraitData.CharacterDescription; }

    // Play greeting sound
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayGreetingSound();
};