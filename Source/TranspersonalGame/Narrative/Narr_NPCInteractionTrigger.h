#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "SharedTypes.h"
#include "Narr_NPCInteractionTrigger.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NPCProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    ENarr_NPCPersonality Personality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    ENarr_NPCRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    TArray<FString> KnownInformation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    float TrustLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    bool bIsAvailable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FString BackgroundStory;

    FNarr_NPCProfile()
    {
        NPCName = TEXT("Unknown");
        Personality = ENarr_NPCPersonality::Neutral;
        Role = ENarr_NPCRole::Villager;
        TrustLevel = 0.5f;
        bIsAvailable = true;
        BackgroundStory = TEXT("A survivor in the prehistoric world.");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_InteractionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FText InteractionPrompt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    ENarr_InteractionType InteractionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bRequiresItem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FString RequiredItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    TArray<FString> PossibleOutcomes;

    FNarr_InteractionData()
    {
        InteractionPrompt = FText::FromString(TEXT("Interact"));
        InteractionType = ENarr_InteractionType::Talk;
        InteractionDuration = 2.0f;
        bRequiresItem = false;
        RequiredItemName = TEXT("");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_NPCInteractionTrigger : public AActor
{
    GENERATED_BODY()

public:
    ANarr_NPCInteractionTrigger();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteractionTrigger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* NPCMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* VoiceAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FNarr_NPCProfile NPCProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FNarr_InteractionData InteractionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bShowInteractionPrompt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<FString> VoiceLineURLs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bShouldAnimate;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void TriggerInteraction(AActor* InteractingActor);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool CanInteract(AActor* InteractingActor);

    UFUNCTION(BlueprintCallable, Category = "NPC")
    void SetNPCProfile(const FNarr_NPCProfile& NewProfile);

    UFUNCTION(BlueprintCallable, Category = "NPC")
    FNarr_NPCProfile GetNPCProfile() const;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void SetInteractionData(const FNarr_InteractionData& NewData);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayVoiceLine(int32 LineIndex);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void AddVoiceLine(const FString& VoiceURL);

    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
    void OnInteractionTriggered(AActor* InteractingActor);

    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
    void OnPlayerEnterRange(AActor* PlayerActor);

    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
    void OnPlayerExitRange(AActor* PlayerActor);

protected:
    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    bool bPlayerInRange;
    AActor* CurrentPlayer;
    float AnimationTimer;
    
    void UpdateNPCAnimation(float DeltaTime);
    void UpdateInteractionPrompt();
    FString GetPersonalityBasedGreeting();
};