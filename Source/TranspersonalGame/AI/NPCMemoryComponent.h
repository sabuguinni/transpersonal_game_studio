#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "NPCMemoryComponent.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    AActor* Actor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastKnownLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastSeenTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bIsFriendly;

    FNPC_MemoryEntry()
    {
        Actor = nullptr;
        LastKnownLocation = FVector::ZeroVector;
        ThreatLevel = 0.0f;
        LastSeenTime = 0.0f;
        bIsFriendly = false;
    }
};

UENUM(BlueprintType)
enum class ENPC_MemoryType : uint8
{
    Threat      UMETA(DisplayName = "Threat"),
    Ally        UMETA(DisplayName = "Ally"),
    Resource    UMETA(DisplayName = "Resource"),
    Location    UMETA(DisplayName = "Location"),
    Event       UMETA(DisplayName = "Event")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCMemoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCMemoryComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Memory storage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FNPC_MemoryEntry> MemoryEntries;

    // Memory settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    float MaxMemoryDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    int32 MaxMemoryEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    float MemoryDecayRate;

    // Memory functions
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddMemoryEntry(AActor* Actor, FVector Location, float ThreatLevel, bool bIsFriendly);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void UpdateMemoryEntry(AActor* Actor, FVector NewLocation, float NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void RemoveMemoryEntry(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    FNPC_MemoryEntry* GetMemoryEntry(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    TArray<FNPC_MemoryEntry> GetMemoriesByType(ENPC_MemoryType MemoryType);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    TArray<FNPC_MemoryEntry> GetNearbyMemories(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    AActor* GetHighestThreatInMemory();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ClearOldMemories();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void DecayMemories(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    bool HasMemoryOf(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    float GetThreatLevelOf(AActor* Actor);
};