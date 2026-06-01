#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "../SharedTypes.h"
#include "Narr_DinosaurNarrator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DinosaurNarration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Narration")
    EDinosaurSpecies DinosaurType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Narration")
    FString NarrationText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Narration")
    USoundCue* VoiceClip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Narration")
    float TriggerDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Narration")
    ENarr_EmotionalTone EmotionalTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Narration")
    bool bPlayOnce;

    FNarr_DinosaurNarration()
    {
        DinosaurType = EDinosaurSpecies::TRex;
        NarrationText = TEXT("");
        VoiceClip = nullptr;
        TriggerDistance = 2000.0f;
        EmotionalTone = ENarr_EmotionalTone::Neutral;
        bPlayOnce = true;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDinosaurNarrationTriggered, EDinosaurSpecies, DinosaurType, const FString&, NarrationText);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_DinosaurNarrator : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DinosaurNarrator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Narration")
    void TriggerNarrationForDinosaur(EDinosaurSpecies DinosaurType, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Narration")
    void AddDinosaurNarration(const FNarr_DinosaurNarration& NewNarration);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Narration")
    void SetNarrationEnabled(bool bEnabled) { bNarrationEnabled = bEnabled; }

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Narration")
    bool IsNarrationEnabled() const { return bNarrationEnabled; }

    UPROPERTY(BlueprintAssignable, Category = "Dinosaur Events")
    FOnDinosaurNarrationTriggered OnDinosaurNarrationTriggered;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* NarrationAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Narration")
    TArray<FNarr_DinosaurNarration> DinosaurNarrations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Narration")
    bool bNarrationEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Narration")
    float PlayerDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Narration")
    float NarrationCooldown;

    UPROPERTY()
    TMap<EDinosaurSpecies, bool> PlayedNarrations;

    UPROPERTY()
    float LastNarrationTime;

    void LoadDefaultDinosaurNarrations();
    bool CanPlayNarration(EDinosaurSpecies DinosaurType) const;
    void PlayNarration(const FNarr_DinosaurNarration& Narration);
    TArray<AActor*> FindNearbyDinosaurs() const;
};