#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorTypes.h"
#include "NPCMemorySystem.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCMemorySystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCMemorySystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Memory management
    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void AddMemoryEntry(AActor* Target, const FVector& Location, float ThreatLevel, bool bIsHostile);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void UpdateMemoryEntry(AActor* Target, const FVector& NewLocation);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void RemoveMemoryEntry(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    FNPC_MemoryEntry GetMemoryEntry(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    bool HasMemoryOf(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    TArray<FNPC_MemoryEntry> GetAllMemories();

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    TArray<FNPC_MemoryEntry> GetHostileMemories();

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void ClearOldMemories(float MaxAge = 300.0f);

    // Threat assessment
    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    float CalculateThreatLevel(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    AActor* GetHighestThreatTarget();

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    FVector GetLastKnownLocation(AActor* Target);

    // Memory sharing between NPCs
    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void ShareMemoryWith(UNPCMemorySystem* OtherNPC, float ShareRadius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void ReceiveSharedMemory(const FNPC_MemoryEntry& SharedMemory);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    float MemoryDecayRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    float MaxMemoryAge = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    int32 MaxMemoryEntries = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    float MemoryUpdateInterval = 1.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory Data")
    TArray<FNPC_MemoryEntry> MemoryEntries;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory Data")
    float LastMemoryUpdate = 0.0f;

private:
    void UpdateMemoryDecay(float DeltaTime);
    void CleanupOldMemories();
    int32 FindMemoryIndex(AActor* Target);
};