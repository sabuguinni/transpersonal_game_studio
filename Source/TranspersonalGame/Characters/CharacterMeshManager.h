#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Animation/AnimBlueprint.h"
#include "CharacterMeshManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_CharacterAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<USkeletalMesh> BaseMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<UMaterialInterface> SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<UAnimBlueprint> AnimationBlueprint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FString TribalAffiliation;

    FChar_CharacterAppearance()
    {
        CharacterName = TEXT("Unnamed");
        TribalAffiliation = TEXT("None");
    }
};

UENUM(BlueprintType)
enum class EChar_CharacterType : uint8
{
    Player          UMETA(DisplayName = "Player Character"),
    TribalHunter    UMETA(DisplayName = "Tribal Hunter"),
    TribalGatherer  UMETA(DisplayName = "Tribal Gatherer"),
    TribalElder     UMETA(DisplayName = "Tribal Elder"),
    TribalChild     UMETA(DisplayName = "Tribal Child"),
    Wanderer        UMETA(DisplayName = "Wanderer"),
    Survivor        UMETA(DisplayName = "Survivor")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCharacterMeshManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCharacterMeshManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Character appearance management
    UFUNCTION(BlueprintCallable, Category = "Character Mesh")
    void SetCharacterAppearance(const FChar_CharacterAppearance& NewAppearance);

    UFUNCTION(BlueprintCallable, Category = "Character Mesh")
    void ApplyTribalVariation(EChar_CharacterType CharacterType);

    UFUNCTION(BlueprintCallable, Category = "Character Mesh")
    void SetSkinMaterial(UMaterialInterface* NewSkinMaterial);

    UFUNCTION(BlueprintCallable, Category = "Character Mesh")
    void SetClothingMaterial(UMaterialInterface* NewClothingMaterial);

    UFUNCTION(BlueprintCallable, Category = "Character Mesh")
    void RandomizeAppearance();

    // Mesh utility functions
    UFUNCTION(BlueprintCallable, Category = "Character Mesh")
    bool LoadDefaultMannequinMesh();

    UFUNCTION(BlueprintCallable, Category = "Character Mesh")
    void UpdateCharacterMesh();

    UFUNCTION(BlueprintPure, Category = "Character Mesh")
    FChar_CharacterAppearance GetCurrentAppearance() const { return CurrentAppearance; }

    UFUNCTION(BlueprintPure, Category = "Character Mesh")
    EChar_CharacterType GetCharacterType() const { return CharacterType; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Mesh")
    FChar_CharacterAppearance CurrentAppearance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Mesh")
    EChar_CharacterType CharacterType;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Mesh")
    class USkeletalMeshComponent* TargetMeshComponent;

    // Tribal appearance presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Presets")
    TArray<FChar_CharacterAppearance> TribalHunterPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Presets")
    TArray<FChar_CharacterAppearance> TribalGathererPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Presets")
    TArray<FChar_CharacterAppearance> TribalElderPresets;

private:
    void InitializeTribalPresets();
    void FindTargetMeshComponent();
    FChar_CharacterAppearance CreateRandomTribalAppearance(EChar_CharacterType Type);
};