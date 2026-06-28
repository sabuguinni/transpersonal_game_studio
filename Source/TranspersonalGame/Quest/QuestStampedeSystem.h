#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "QuestTypes.h"
#include "QuestStampedeSystem.generated.h"

// ============================================================
// EQuest_StampedePhase — phases of the Survive the Stampede quest
// ============================================================
UENUM(BlueprintType)
enum class EQuest_StampedePhase : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Triggered       UMETA(DisplayName = "Triggered"),       // Player entered danger zone
    Escaping        UMETA(DisplayName = "Escaping"),        // Countdown active
    Escaped         UMETA(DisplayName = "Escaped"),         // Reached safe zone in time
    Crushed         UMETA(DisplayName = "Crushed"),         // Failed — time ran out
    Completed       UMETA(DisplayName = "Completed")        // Reward collected
};

// ============================================================
// FQuest_StampedeObjective — single objective entry
// ============================================================
USTRUCT(BlueprintType)
struct FQuest_StampedeObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Stampede")
    FString ObjectiveName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Stampede")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Stampede")
    bool bCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Stampede")
    bool bOptional = false;
};

// ============================================================
// AQuestStampedeTrigger — overlap box that starts the quest
// ============================================================
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuestStampedeTrigger : public AActor
{
    GENERATED_BODY()

public:
    AQuestStampedeTrigger();

    // --- Components ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Stampede",
        meta = (AllowPrivateAccess = "true"))
    UBoxComponent* TriggerVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Stampede",
        meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* DebugMarker;

    // --- Quest State ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Stampede")
    EQuest_StampedePhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Stampede")
    float EscapeTimeLimit = 60.0f;          // seconds to reach safe zone

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Stampede")
    float RemainingTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Stampede")
    FVector SafeZoneLocation = FVector(3000.0f, -1500.0f, 400.0f);  // High ground

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Stampede")
    float SafeZoneRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Stampede")
    bool bQuestAlreadyCompleted = false;

    // --- Objectives ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Stampede")
    TArray<FQuest_StampedeObjective> Objectives;

    // --- Audio ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Stampede|Audio")
    FString TriggerVoiceURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782653080550_QuestNPC_Elder.mp3";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Stampede|Audio")
    FString CompletionVoiceURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782653095161_QuestNPC_Elder.mp3";

    // --- UE5 Lifecycle ---
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- Quest API ---
    UFUNCTION(BlueprintCallable, Category = "Quest|Stampede")
    void ActivateStampedeQuest(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Quest|Stampede")
    void CheckSafeZoneReached(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Quest|Stampede")
    void CompleteObjective(int32 ObjectiveIndex);

    UFUNCTION(BlueprintCallable, Category = "Quest|Stampede")
    void FailQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest|Stampede")
    void CompleteQuest();

    UFUNCTION(BlueprintPure, Category = "Quest|Stampede")
    float GetEscapeProgressPercent() const;

    UFUNCTION(BlueprintPure, Category = "Quest|Stampede")
    bool IsPlayerInSafeZone(AActor* PlayerActor) const;

    UFUNCTION(CallInEditor, Category = "Quest|Stampede")
    void ResetQuest();

private:
    UFUNCTION()
    void OnTriggerOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    void InitialiseObjectives();
    bool bTickActive = false;
};

// ============================================================
// AQuestSafeZoneMarker — destination actor on high ground
// ============================================================
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuestSafeZoneMarker : public AActor
{
    GENERATED_BODY()

public:
    AQuestSafeZoneMarker();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|SafeZone",
        meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* MarkerMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|SafeZone",
        meta = (AllowPrivateAccess = "true"))
    UBoxComponent* SafeVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|SafeZone")
    bool bIsActive = false;

    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category = "Quest|SafeZone")
    void ActivateMarker();

    UFUNCTION(BlueprintCallable, Category = "Quest|SafeZone")
    void DeactivateMarker();

private:
    UFUNCTION()
    void OnSafeZoneOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};
