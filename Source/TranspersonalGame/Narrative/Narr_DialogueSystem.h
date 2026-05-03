#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerBox.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "../Core/SharedTypes.h"
#include "Narr_DialogueSystem.generated.h"

/**
 * NARRATIVE AGENT #15 - DYNAMIC DIALOGUE SYSTEM
 * 
 * Sistema de diálogo dinâmico baseado em contexto e proximidade.
 * Reproduz narrativa contextual baseada na localização do jogador,
 * presença de dinossauros e estado de sobrevivência.
 */

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Tutorial = 0        UMETA(DisplayName = "Tutorial"),
    Warning = 1         UMETA(DisplayName = "Warning"),
    Discovery = 2       UMETA(DisplayName = "Discovery"),
    Survival = 3        UMETA(DisplayName = "Survival"),
    Research = 4        UMETA(DisplayName = "Research"),
    Atmosphere = 5      UMETA(DisplayName = "Atmosphere")
};

UENUM(BlueprintType)
enum class ENarr_NarrativeContext : uint8
{
    Safe = 0            UMETA(DisplayName = "Safe"),
    Danger = 1          UMETA(DisplayName = "Danger"),
    Exploration = 2     UMETA(DisplayName = "Exploration"),
    Combat = 3          UMETA(DisplayName = "Combat"),
    Discovery = 4       UMETA(DisplayName = "Discovery"),
    Survival = 5        UMETA(DisplayName = "Survival")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType DialogueType = ENarr_DialogueType::Tutorial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TSoftObjectPtr<USoundCue> AudioClip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 Priority = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bCanRepeat = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float CooldownTime = 30.0f;

    FNarr_DialogueEntry()
    {
        DialogueType = ENarr_DialogueType::Tutorial;
        DialogueText = TEXT("");
        Duration = 10.0f;
        Priority = 1;
        bCanRepeat = false;
        CooldownTime = 30.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NarrativeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    ENarr_NarrativeContext Context = ENarr_NarrativeContext::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    TArray<FNarr_DialogueEntry> AvailableDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FVector ZoneCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    float ZoneRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    bool bIsActive = true;

    FNarr_NarrativeZone()
    {
        ZoneName = TEXT("");
        Context = ENarr_NarrativeContext::Safe;
        ZoneCenter = FVector::ZeroVector;
        ZoneRadius = 1000.0f;
        bIsActive = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Sistema de zonas narrativas
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Zones")
    TArray<FNarr_NarrativeZone> NarrativeZones;

    // Sistema de diálogo activo
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Dialogue")
    FNarr_DialogueEntry CurrentDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Dialogue")
    bool bIsPlayingDialogue = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Dialogue")
    float DialogueTimer = 0.0f;

    // Configurações do sistema
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float UpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MaxDialogueDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableContextualDialogue = true;

    // Componente de áudio para reprodução
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* AudioComponent;

    // Métodos públicos
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue(ENarr_DialogueType DialogueType, const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsInNarrativeZone(const FVector& PlayerLocation, FString& OutZoneName);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AddNarrativeZone(const FNarr_NarrativeZone& NewZone);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RemoveNarrativeZone(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    ENarr_NarrativeContext GetCurrentContext() const;

private:
    // Métodos internos
    void UpdateNarrativeContext();
    void ProcessDialogueQueue();
    FNarr_DialogueEntry SelectBestDialogue(const FNarr_NarrativeZone& Zone);
    bool ShouldPlayDialogue(const FNarr_DialogueEntry& Dialogue);
    
    // Estado interno
    float LastUpdateTime = 0.0f;
    ENarr_NarrativeContext CurrentContext = ENarr_NarrativeContext::Safe;
    FString CurrentZone;
    TMap<FString, float> DialogueCooldowns;
};