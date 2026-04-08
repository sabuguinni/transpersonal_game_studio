#include "AnimationSystemManager.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "PoseSearch/PoseSearchLibrary.h"
#include "IKRig/IKRigProcessor.h"
#include "Kismet/KismetMathLibrary.h"

UAnimationSystemManager::UAnimationSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Configuração padrão
    DinosaurVariationMultiplier = 1.0f;
    VariationSeed = 12345;
}

void UAnimationSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    SetupTrajectorySystem();
    InitializePoseSearchDatabases();
    
    UE_LOG(LogAnimation, Log, TEXT("Animation System Manager initialized for Transpersonal Game"));
}

void UAnimationSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Actualiza todos os sistemas IK activos
    for (auto& IKPair : ActiveIKProcessors)
    {
        if (IsValid(IKPair.Key) && IsValid(IKPair.Value))
        {
            UpdateTerrainAdaptation(IKPair.Key, DeltaTime);
        }
    }
}

bool UAnimationSystemManager::InitializeMotionMatching(AActor* Character, ECharacterType CharacterType)
{
    if (!IsValid(Character))
    {
        UE_LOG(LogAnimation, Warning, TEXT("Invalid character passed to InitializeMotionMatching"));
        return false;
    }
    
    USkeletalMeshComponent* SkeletalMesh = Character->FindComponentByClass<USkeletalMeshComponent>();
    if (!IsValid(SkeletalMesh))
    {
        UE_LOG(LogAnimation, Warning, TEXT("Character has no SkeletalMeshComponent"));
        return false;
    }
    
    // Selecciona o schema apropriado baseado no tipo de personagem
    UPoseSearchSchema* SelectedSchema = nullptr;
    UPoseSearchDatabase* SelectedDatabase = nullptr;
    
    switch (CharacterType)
    {
        case ECharacterType::Protagonist:
            SelectedSchema = ProtagonistMotionSchema;
            SelectedDatabase = ProtagonistAnimDatabase;
            break;
            
        case ECharacterType::SmallHerbivore:
        case ECharacterType::LargeHerbivore:
            SelectedSchema = SmallHerbivoreSchema;
            break;
            
        case ECharacterType::SmallCarnivore:
        case ECharacterType::LargeCarnivore:
            SelectedSchema = CarnivoreSchema;
            break;
            
        default:
            UE_LOG(LogAnimation, Warning, TEXT("Unsupported character type for Motion Matching"));
            return false;
    }
    
    if (!IsValid(SelectedSchema))
    {
        UE_LOG(LogAnimation, Error, TEXT("No valid schema found for character type"));
        return false;
    }
    
    // Cria e configura Motion Matching instance
    UMotionMatchingAnimInstance* MotionInstance = NewObject<UMotionMatchingAnimInstance>(Character);
    if (IsValid(MotionInstance))
    {
        // Configuração específica para o protagonista (movimentos cautelosos)
        if (CharacterType == ECharacterType::Protagonist)
        {
            // Reduz velocidade base para movimentos mais científicos/cautelosos
            MotionInstance->SetPlayRate(0.85f);
            
            // Aumenta peso de trajectory samples para movimentos mais previsíveis
            MotionInstance->SetTrajectoryWeight(1.2f);
        }
        
        ActiveMotionMatchingInstances.Add(Character, MotionInstance);
        
        UE_LOG(LogAnimation, Log, TEXT("Motion Matching initialized for character: %s"), 
               *Character->GetName());
        return true;
    }
    
    return false;
}

void UAnimationSystemManager::ApplyDinosaurVariations(AActor* Dinosaur, FString DinosaurID)
{
    if (!IsValid(Dinosaur) || DinosaurID.IsEmpty())
    {
        return;
    }
    
    // Verifica se já temos variações para este dinossauro
    if (DinosaurVariations.Contains(DinosaurID))
    {
        return; // Já foi processado
    }
    
    // Gera variações únicas baseadas no ID do dinossauro
    FRandomStream RandomStream(GetTypeHash(DinosaurID) + VariationSeed);
    
    FDinosaurAnimationVariation NewVariation;
    
    // Variação de velocidade (±20%)
    NewVariation.MovementSpeedModifier = RandomStream.FRandRange(0.8f, 1.2f);
    
    // Variação de postura (-0.3 a +0.3)
    NewVariation.PostureModifier = RandomStream.FRandRange(-0.3f, 0.3f);
    
    // Variação de agressividade (0.0 a 1.0)
    NewVariation.AggressionModifier = RandomStream.FRandRange(0.0f, 1.0f);
    
    // Variação de nervosismo (0.0 a 1.0)
    NewVariation.NervousnessModifier = RandomStream.FRandRange(0.0f, 1.0f);
    
    // Padrão de respiração único (±30%)
    NewVariation.BreathingPattern = RandomStream.FRandRange(0.7f, 1.3f);
    
    // Frequência de movimentos de cabeça (±50%)
    NewVariation.HeadMovementFrequency = RandomStream.FRandRange(0.5f, 1.5f);
    
    // Armazena as variações
    DinosaurVariations.Add(DinosaurID, NewVariation);
    
    // Aplica as variações ao Animation Blueprint do dinossauro
    USkeletalMeshComponent* SkeletalMesh = Dinosaur->FindComponentByClass<USkeletalMeshComponent>();
    if (IsValid(SkeletalMesh) && IsValid(SkeletalMesh->GetAnimInstance()))
    {
        UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance();
        
        // Define variáveis no Animation Blueprint
        AnimInstance->SetVariableValue(FName("MovementSpeedModifier"), NewVariation.MovementSpeedModifier);
        AnimInstance->SetVariableValue(FName("PostureModifier"), NewVariation.PostureModifier);
        AnimInstance->SetVariableValue(FName("AggressionLevel"), NewVariation.AggressionModifier);
        AnimInstance->SetVariableValue(FName("NervousnessLevel"), NewVariation.NervousnessModifier);
        AnimInstance->SetVariableValue(FName("BreathingRate"), NewVariation.BreathingPattern);
        AnimInstance->SetVariableValue(FName("HeadMovementFreq"), NewVariation.HeadMovementFrequency);
    }
    
    UE_LOG(LogAnimation, Log, TEXT("Applied unique variations to dinosaur: %s"), *DinosaurID);
}

void UAnimationSystemManager::UpdateTerrainAdaptation(AActor* Character, float DeltaTime)
{
    if (!IsValid(Character) || !IsValid(TerrainAdaptationRig))
    {
        return;
    }
    
    USkeletalMeshComponent* SkeletalMesh = Character->FindComponentByClass<USkeletalMeshComponent>();
    if (!IsValid(SkeletalMesh))
    {
        return;
    }
    
    // Obtém ou cria IK Processor para este personagem
    UIKRigProcessor** ProcessorPtr = ActiveIKProcessors.Find(Character);
    UIKRigProcessor* Processor = nullptr;
    
    if (ProcessorPtr && IsValid(*ProcessorPtr))
    {
        Processor = *ProcessorPtr;
    }
    else
    {
        Processor = NewObject<UIKRigProcessor>(Character);
        if (IsValid(Processor))
        {
            Processor->SetIKRig(TerrainAdaptationRig);
            ActiveIKProcessors.Add(Character, Processor);
        }
    }
    
    if (!IsValid(Processor))
    {
        return;
    }
    
    // Executa ray casting para os pés
    FVector LeftFootLocation, RightFootLocation;
    FVector LeftFootNormal, RightFootNormal;
    
    bool bLeftFootHit = PerformFootRaycast(Character, TEXT("foot_l"), LeftFootLocation, LeftFootNormal);
    bool bRightFootHit = PerformFootRaycast(Character, TEXT("foot_r"), RightFootLocation, RightFootNormal);
    
    if (bLeftFootHit || bRightFootHit)
    {
        // Actualiza IK goals baseado no terreno
        if (bLeftFootHit)
        {
            Processor->SetGoalTransform(FName("LeftFootIK"), 
                FTransform(LeftFootNormal.ToOrientationQuat(), LeftFootLocation));
        }
        
        if (bRightFootHit)
        {
            Processor->SetGoalTransform(FName("RightFootIK"), 
                FTransform(RightFootNormal.ToOrientationQuat(), RightFootLocation));
        }
        
        // Executa o solve do IK
        Processor->Solve(DeltaTime);
    }
}

float UAnimationSystemManager::CalculateEmotionalBlendWeight(float FearLevel, float CuriosityLevel)
{
    // Fórmula baseada na psicologia do protagonista paleontologista
    // Medo alto = movimentos mais cautelosos e lentos
    // Curiosidade alta = movimentos mais focados e deliberados
    
    float FearWeight = FMath::Clamp(FearLevel, 0.0f, 1.0f);
    float CuriosityWeight = FMath::Clamp(CuriosityLevel, 0.0f, 1.0f);
    
    // Quando o medo é alto, reduz a velocidade e aumenta a cautela
    float CautiousBlend = FearWeight * 0.7f;
    
    // Quando a curiosidade é alta, aumenta a precisão dos movimentos
    float FocusedBlend = CuriosityWeight * 0.3f;
    
    return FMath::Clamp(CautiousBlend + FocusedBlend, 0.1f, 1.0f);
}

void UAnimationSystemManager::SetupTrajectorySystem()
{
    // Configuração do sistema de trajectory para Motion Matching
    // Baseado nas necessidades do jogo de sobrevivência
    
    UE_LOG(LogAnimation, Log, TEXT("Setting up Trajectory System for Motion Matching"));
    
    // Configurações serão aplicadas via Pose Search Schema assets
    // Trajectory samples: passado (-0.5s, -0.25s), presente (0.0s), futuro (+0.25s, +0.5s)
    // Flags: Position, Velocity, Facing Direction
}

void UAnimationSystemManager::InitializePoseSearchDatabases()
{
    // Inicialização das databases de Motion Matching
    // Cada database contém animações específicas para diferentes tipos de personagem
    
    UE_LOG(LogAnimation, Log, TEXT("Initializing Pose Search Databases"));
    
    // Database do protagonista deve conter:
    // - Idle científico (observando, anotando)
    // - Caminhada cautelosa
    // - Corrida de fuga (não atlética)
    // - Agachamento para recolher amostras
    // - Movimentos de escalada básica
}

bool UAnimationSystemManager::PerformFootRaycast(AActor* Character, const FString& FootBoneName, 
                                                FVector& OutLocation, FVector& OutNormal)
{
    if (!IsValid(Character))
    {
        return false;
    }
    
    USkeletalMeshComponent* SkeletalMesh = Character->FindComponentByClass<USkeletalMeshComponent>();
    if (!IsValid(SkeletalMesh))
    {
        return false;
    }
    
    // Obtém a localização do osso do pé
    FVector FootBoneLocation = SkeletalMesh->GetBoneLocation(FName(*FootBoneName));
    
    // Ray cast para baixo
    FVector StartLocation = FootBoneLocation + FVector(0, 0, 50.0f);
    FVector EndLocation = FootBoneLocation - FVector(0, 0, 100.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Character);
    
    bool bHit = Character->GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_WorldStatic,
        QueryParams
    );
    
    if (bHit)
    {
        OutLocation = HitResult.Location;
        OutNormal = HitResult.Normal;
        return true;
    }
    
    return false;
}