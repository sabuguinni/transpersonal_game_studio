#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "NPCMemorySystem.generated.h"

UENUM(BlueprintType)
enum class ENPC_MemoryType : uint8
{
    PlayerSighting,
    DinosaurEncounter,
    ResourceLocation,
    DangerZone,
    SafeArea,
    SocialInteraction,
    WeatherEvent,
    TimeOfDay
};

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENPC_MemoryType MemoryType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Importance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EmotionalImpact;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> AssociatedTags;

    FNPC_MemoryEntry()
    {
        MemoryType = ENPC_MemoryType::PlayerSighting;
        Location = FVector::ZeroVector;
        Timestamp = 0.0f;
        Importance = 0.5f;
        EmotionalImpact = 0.0f;
        Description = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FNPC_EmotionalState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Fear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Curiosity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Aggression;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Trust;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Hunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Fatigue;

    FNPC_EmotionalState()
    {
        Fear = 0.3f;
        Curiosity = 0.5f;
        Aggression = 0.2f;
        Trust = 0.1f;
        Hunger = 0.4f;
        Fatigue = 0.3f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_MemorySystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_MemorySystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Memory Management
    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void AddMemory(ENPC_MemoryType Type, FVector Location, float Importance, const FString& Description);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    TArray<FNPC_MemoryEntry> GetMemoriesByType(ENPC_MemoryType Type);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    TArray<FNPC_MemoryEntry> GetMemoriesNearLocation(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void ForgetOldMemories(float MaxAge);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    bool HasMemoryOfLocation(FVector Location, float Radius, ENPC_MemoryType Type);

    // Emotional State
    UFUNCTION(BlueprintCallable, Category = "NPC Emotions")
    void UpdateEmotionalState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "NPC Emotions")
    void ModifyEmotion(const FString& EmotionName, float Delta);

    UFUNCTION(BlueprintCallable, Category = "NPC Emotions")
    float GetEmotionValue(const FString& EmotionName);

    UFUNCTION(BlueprintCallable, Category = "NPC Emotions")
    FNPC_EmotionalState GetCurrentEmotionalState() const { return CurrentEmotionalState; }

    // Decision Making
    UFUNCTION(BlueprintCallable, Category = "NPC Decision")
    bool ShouldFleeFromLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "NPC Decision")
    bool ShouldInvestigateLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "NPC Decision")
    FVector GetSafestKnownLocation();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    int32 MaxMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    float MemoryDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    float EmotionalDecayRate;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory Data")
    TArray<FNPC_MemoryEntry> Memories;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Emotional Data")
    FNPC_EmotionalState CurrentEmotionalState;

private:
    void CleanupOldMemories();
    void DecayEmotions(float DeltaTime);
    float CalculateMemoryImportance(const FNPC_MemoryEntry& Memory);
};