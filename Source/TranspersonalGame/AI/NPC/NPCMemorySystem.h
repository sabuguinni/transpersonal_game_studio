#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "NPCMemorySystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    ENPC_MemoryType MemoryType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Importance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    AActor* RelatedActor;

    FNPC_MemoryEntry()
    {
        Location = FVector::ZeroVector;
        Timestamp = 0.0f;
        MemoryType = ENPC_MemoryType::Neutral;
        Importance = 1.0f;
        RelatedActor = nullptr;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    int32 MaxMemoryEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    float MemoryDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    float ForgetThreshold;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory Data")
    TArray<FNPC_MemoryEntry> MemoryEntries;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddMemory(FVector Location, ENPC_MemoryType Type, float Importance, AActor* Actor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    TArray<FNPC_MemoryEntry> GetMemoriesOfType(ENPC_MemoryType Type);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    FNPC_MemoryEntry GetMostImportantMemory();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    bool HasMemoryOfActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ForgetOldMemories();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ClearAllMemories();
};