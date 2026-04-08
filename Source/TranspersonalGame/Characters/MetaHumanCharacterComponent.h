#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "CharacterSystem.h"
#include "MetaHumanCharacterComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterLoaded, const FCharacterDefinition&, Character);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UMetaHumanCharacterComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMetaHumanCharacterComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Character Definition
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FCharacterDefinition CharacterDefinition;

    // MetaHuman Components
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    USkeletalMeshComponent* BodyMeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    USkeletalMeshComponent* FaceMeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    USkeletalMeshComponent* HairMeshComponent;

    // Clothing Components
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    USkeletalMeshComponent* HeadGearComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    USkeletalMeshComponent* TorsoComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    USkeletalMeshComponent* ArmsComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    USkeletalMeshComponent* LegsComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    USkeletalMeshComponent* FeetComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    USkeletalMeshComponent* AccessoriesComponent;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCharacterLoaded OnCharacterLoaded;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Character")
    void LoadCharacter(const FCharacterDefinition& NewCharacterDefinition);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void LoadCharacterByName(const FString& CharacterName);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void LoadRandomCharacterByArchetype(ECharacterArchetype Archetype);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void ApplyPhysicalTraits(const FCharacterPhysicalTraits& Traits);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void ApplyClothing(const FCharacterClothing& Clothing);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void SetupMetaHumanComponents();

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void SetMasterPoseComponent(USkeletalMeshComponent* MasterComponent);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Database")
    TSoftObjectPtr<UCharacterDatabase> CharacterDatabase;

    UFUNCTION()
    void OnMetaHumanMeshLoaded();

    UFUNCTION()
    void OnClothingMeshLoaded();

private:
    bool bIsLoading = false;
    int32 PendingMeshLoads = 0;

    void IncrementPendingLoads();
    void DecrementPendingLoads();
    void CheckLoadingComplete();
};