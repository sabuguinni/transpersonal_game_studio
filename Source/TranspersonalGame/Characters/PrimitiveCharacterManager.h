#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "TranspersonalGame/SharedTypes.h"
#include "PrimitiveCharacterManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_PrimitiveAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float SkinTone = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float BodyBuild = 0.5f; // 0=lean, 1=muscular

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Age = 0.3f; // 0=young, 1=elder

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    int32 ScarPattern = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    int32 HairStyle = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor ClothingColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_CharacterArchetype
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    FString ArchetypeName = "Hunter";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    FChar_PrimitiveAppearance DefaultAppearance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    TArray<FString> PreferredEquipment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    float StrengthBonus = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    float AgilityBonus = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    float IntelligenceBonus = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APrimitiveCharacterManager : public AActor
{
    GENERATED_BODY()

public:
    APrimitiveCharacterManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character System")
    TArray<FChar_CharacterArchetype> AvailableArchetypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character System")
    TArray<TSoftObjectPtr<USkeletalMesh>> PrimitiveMeshVariants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character System")
    TSoftObjectPtr<UMaterialInterface> BaseSkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character System")
    TSoftObjectPtr<UMaterialInterface> BaseClothingMaterial;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<ACharacter*> SpawnedCharacters;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    ACharacter* CreatePrimitiveCharacter(const FChar_CharacterArchetype& Archetype, 
                                       const FVector& SpawnLocation, 
                                       const FRotator& SpawnRotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    void ApplyAppearanceToCharacter(ACharacter* Character, const FChar_PrimitiveAppearance& Appearance);

    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    void SetupCharacterForSurvival(ACharacter* Character, const FChar_CharacterArchetype& Archetype);

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    TArray<ACharacter*> GetAllManagedCharacters() const { return SpawnedCharacters; }

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    void RemoveCharacter(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Archetypes")
    FChar_CharacterArchetype GetArchetypeByName(const FString& Name) const;

    UFUNCTION(BlueprintCallable, Category = "Archetypes")
    TArray<FString> GetAvailableArchetypeNames() const;

    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void InitializeDefaultArchetypes();

    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void SpawnTestCharacters();

protected:
    void SetupDefaultArchetypes();
    UMaterialInstanceDynamic* CreateCustomSkinMaterial(const FChar_PrimitiveAppearance& Appearance);
    UMaterialInstanceDynamic* CreateCustomClothingMaterial(const FChar_PrimitiveAppearance& Appearance);
    void ApplyArchetypeStats(ACharacter* Character, const FChar_CharacterArchetype& Archetype);
};